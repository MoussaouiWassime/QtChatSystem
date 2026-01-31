#include <QMessageBox>
#include "Chat.h"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// Chat ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Processeurs.
const std::map<QString, Chat::Processor> Chat::PROCESSORS {
    {"#alias",        &Chat::process_alias},
    {"#connected",    &Chat::process_connected},
    {"#disconnected", &Chat::process_disconnected},
    {"#renamed",      &Chat::process_renamed},
    {"#list",         &Chat::process_list},
    {"#private",      &Chat::process_private},
    {"#error",        &Chat::process_error}
};

// Constructeur.
Chat::Chat (const QString & host, quint16 port, QObject * parent) :
  QObject (parent),
  socket ()
{
    // Signal "connected" émis lorsque la connexion est effectuée.
    connect (&socket, &QTcpSocket::connected, [this, host, port] () {
        emit connected (host, port);
    });

    // Signal "disconnected" émis lors d'une déconnexion du socket.
    connect (&socket, &QTcpSocket::disconnected, this, &Chat::disconnected);

    // Lecture.
    connect (&socket, &QIODevice::readyRead, [this] () {
        // Tant que l'on peut lire une ligne...
        while (socket.canReadLine ())
        {
            // Lecture d'une ligne et suppression du "newline".
            QString m = socket.readLine ().chopped (1);

            // Flot de lecture.
            QTextStream stream (&m);
            // Lecture d'une commande potentielle.
            QString command;
            stream >> command;

            // Recherche de la commande serveur dans le tableau associatif.
            std::map<QString, Chat::Processor>::const_iterator it = PROCESSORS.find (command);

            if (it != PROCESSORS.end ())
            {
                // - si elle existe, traitement du reste du message par le processeur.
                Processor processor = it->second;
                (this->*processor) (stream);
            }
            else
            {
                // - sinon, émission du signal "message" contenant la ligne entière.
                emit message (m);
            }
        }
    });

    // CONNEXION !
    socket.connectToHost (host, port, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
}

Chat::~Chat ()
{
    // Déconnexion des signaux.
    socket.disconnect ();
}

// Commande "#alias"
void Chat::process_alias (QTextStream & is)
{
    QString pseudo;
    is >> pseudo;
    emit alias (pseudo);
}

// Commande "#connected"
void Chat::process_connected (QTextStream & is)
{
    QString pseudo;
    is >> pseudo;
    emit user_connected (pseudo);
}

// Commande "#disconnected"
void Chat::process_disconnected (QTextStream & is)
{
    QString pseudo;
    is >> pseudo;
    emit user_disconnected (pseudo);
}

// Commande "#renamed"
void Chat::process_renamed (QTextStream & is)
{
    QString oldPseudo, newPseudo;
    is >> oldPseudo >> newPseudo;
    emit user_renamed (oldPseudo, newPseudo);
}

// Commande "#list"
void Chat::process_list (QTextStream & is)
{
    QStringList pseudos;
    while (!is.atEnd ())
    {
        QString pseudo;
        is >> pseudo;
        if (!pseudo.isEmpty()) {
            pseudos << pseudo;
        }
    }
    emit user_list (pseudos);
}

// Commande "#private"
void Chat::process_private (QTextStream & is)
{
    QString sender;
    is >> sender;
    QString msg = is.readAll ().trimmed ();
    emit user_private (sender, msg);
}

// Commande "#error"
void Chat::process_error (QTextStream & is)
{
    QString id;
    is >> id >> Qt::ws;
    emit error (id);
}

// Envoi d'un message à travers le socket.
void Chat::write (const QString & message)
{
    socket.write (message.toUtf8 () + '\n');
}

////////////////////////////////////////////////////////////////////////////////
// ChatWindow //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ChatWindow::ChatWindow (const QString & host, quint16 port, QWidget * parent) :
    QMainWindow (parent),
    chat (host, port, this),
    text (this),
    input (this),
    users(this)
{
    text.setReadOnly (true);
    setCentralWidget (&text);

    // Insertion de la zone de saisie.
    // QDockWidget insérable en haut ou en bas, inséré en bas.
    QDockWidget * dock = new QDockWidget (tr("Message"), this);
    dock->setAllowedAreas (Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dock->setWidget (&input);
    addDockWidget (Qt::BottomDockWidgetArea, dock);

    QDockWidget * userDock = new QDockWidget (tr("Users"), this);
    userDock->setAllowedAreas (Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    userDock->setWidget (&users);
    addDockWidget (Qt::RightDockWidgetArea, userDock);

    // Désactivation de la zone de saisie.
    input.setEnabled (false);

    // Envoi de messages lorsque la touche "entrée" est pressée.
    // - transmission du texte au moteur de messagerie instantanée ;
    // - effacement de la zone de saisie.
    connect (&input, &QLineEdit::returnPressed, [this] () {
        QString text = input.text ();
        if (!text.isEmpty())
        {
            chat.write (text);
            input.clear ();
        }
    });

    // Connexion.
    // - affichage d'un message confirmant la connexion ;
    // - saisie de l'alias ;
    // - envoi de l'alias ;
    // - activation de la zone de saisie.
    connect (&chat, &Chat::connected, [this] (const QString & host, quint16 port) {
        text.append (tr("<b>Connected to %1:%2</b>").arg(host).arg(port));

        bool ok;
        QString pseudo = QInputDialog::getText (this, tr("Alias"), tr("Choose an alias:"), QLineEdit::Normal, QString(), &ok);

        if (ok && !pseudo.isEmpty())
        {
            chat.write (pseudo);

            input.setEnabled (true);
            input.setFocus ();
        }
    });

    // Déconnexion.
    // - désactivation de la zone de saisie.
    // - affichage d'un message pour signaler la déconnexion.
    connect (&chat, &Chat::disconnected, [this] () {
        input.setEnabled (false);
        text.append (tr("<b>Disconnected</b>"));
    });

    // Messages.
    connect (&chat, &Chat::message, [this] (const QString & message) {
        text.append (message);
    });

    // Liste des utilisateurs.
    // Connexion d'un utilisateur.
    // Déconnexion d'un utilisateur.
    // Nouvel alias d'un utilisateur.
    // Message privé.

    connect (&chat, &Chat::alias, [this] (const QString & pseudo) {
        this->setWindowTitle(pseudo);
        text.append (tr("<em>Alias validated: <strong>%1</strong></em>").arg(pseudo));
    });

    connect (&chat, &Chat::user_list, [this] (const QStringList & pseudos) {
        users.clear();
        users.addItems(pseudos);
        text.append (tr("<em>Connected users: %1</em>").arg(pseudos.join(", ")));
    });

    connect (&chat, &Chat::user_connected, [this] (const QString & pseudo) {
        users.addItem(pseudo);
        text.append (tr("<em>%1 has joined the chat.</em>").arg(pseudo));
    });

    connect (&chat, &Chat::user_disconnected, [this] (const QString & pseudo) {
        QList<QListWidgetItem *> items = users.findItems(pseudo, Qt::MatchExactly);
        qDeleteAll(items);
        text.append (tr("<em>%1 has left.</em>").arg(pseudo));
    });

    connect (&chat, &Chat::user_renamed, [this] (const QString & oldPseudo, const QString & newPseudo) {
        QList<QListWidgetItem *> items = users.findItems(oldPseudo, Qt::MatchExactly);
        if (!items.isEmpty()) {
            items.first()->setText(newPseudo);
        }
        text.append (tr("<em>%1 is now known as %2.</em>").arg(oldPseudo, newPseudo));
      });

    connect (&chat, &Chat::user_private, [this] (const QString & sender, const QString & message) {
        text.append (tr("<font color='blue'>[Private from %1]: %2</font>").arg(sender, message));
    });

    // Gestion des erreurs.
    connect (&chat, &Chat::error, [this] (const QString & id) {
        QMessageBox::critical (this, tr("Error"), id);
    });

    connect (&users, &QListWidget::itemDoubleClicked, [this] (QListWidgetItem * item) {
        QString targetUser = item->text();
        bool ok;
        QString msg = QInputDialog::getText(this,
                                            tr("Message Privé"),
                                            tr("Message pour %1 :").arg(targetUser),
                                            QLineEdit::Normal,
                                            QString(),
                                            &ok);

        if (ok && !msg.isEmpty()) {
            chat.write("/private " + targetUser + " " + msg);
        }
    });

    // CONNEXION !
    text.append (tr("<b>Connecting...</b>"));
}
