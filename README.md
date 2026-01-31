# ChatCPP - Application de messagerie instantanée

Application de messagerie composée d'un **client** avec interface graphique Qt et d'un **serveur**.

## Description du projet

Ce projet est une application de messagerie instantanée client-serveur développée en C++. 

### Fonctionnalités

- **Connexion/Déconnexion** : Les utilisateurs peuvent se connecter et se déconnecter du serveur
- **Messages publics** : Envoi de messages visibles par tous les utilisateurs connectés
- **Messages privés** : Envoi de messages privés à un utilisateur spécifique (`/private <pseudo> <message>`)
- **Changement de pseudo** : Possibilité de changer son pseudo (`/alias <nouveau_pseudo>`)
- **Liste des utilisateurs** : Affichage en temps réel des utilisateurs connectés
- **Notifications** : Notification lors de la connexion/déconnexion d'utilisateurs

## Prérequis

### Pour le serveur
- **Compilateur C++** : g++ avec support C++14 ou supérieur
- **ASIO** : Bibliothèque réseau (incluse dans le projet, version 1.12.2)
- **Windows** : Bibliothèques `ws2_32` et `mswsock` (socket Windows)

### Pour le client
- **Qt 6** (ou Qt 5 compatible) avec les modules :
  - `core`
  - `gui`
  - `widgets`
  - `network`
- **qmake** ou **Qt Creator**

## Compilation

### Serveur

Depuis le dossier `chat-server/` :

```bash
# Avec Make
make

# Ou manuellement avec g++
g++ -std=c++14 -DASIO_STANDALONE -Iasio-asio-1-12-2/asio/include -pthread main.cpp -o server.exe -lws2_32 -lmswsock
```

### Client

Depuis le dossier `chat-client/` :

```bash
# Avec qmake
qmake Chat.pro
make

# Ou ouvrir Chat.pro avec Qt Creator et compiler
```

## ▶️ Lancement

### 1. Démarrer le serveur

```bash
cd chat-server
./server.exe <port>

# Exemple :
./server.exe 3101
```

Le serveur écoute sur le port spécifié et affiche les connexions entrantes.

### 2. Démarrer le(s) client(s)

```bash
cd chat-client/build/Desktop_Qt_6_10_1_MinGW_64_bit-Debug/debug
./Chat.exe
```

Ou lancez directement depuis Qt Creator.

> **Note** : Par défaut, le client se connecte à `127.0.0.1:3101`. Pour modifier l'adresse ou le port, éditez le fichier [chat-client/main.cpp](chat-client/main.cpp#L12).

## Commandes disponibles (côté client)

| Commande | Description |
|----------|-------------|
| `/alias <pseudo>` | Change votre pseudo |
| `/private <pseudo> <message>` | Envoie un message privé |
| `/list` | Affiche la liste des utilisateurs connectés |
| `/quit` | Quitte le chat |

## Structure du projet

```
chatCPP/
├── chat-client/           # Client Qt
│   ├── Chat.cpp           # Logique du chat (connexion, messages)
│   ├── Chat.h             # Classes Chat et ChatWindow
│   ├── main.cpp           # Point d'entrée du client
│   ├── Chat.pro           # Fichier projet Qt
│   └── build/             # Dossiers de compilation
│
├── chat-server/           # Serveur ASIO
│   ├── main.cpp           # Point d'entrée du serveur
│   ├── server.hpp         # Classe Server et gestion des clients
│   ├── Makefile           # Fichier de compilation
│   └── asio-asio-1-12-2/  # Bibliothèque ASIO standalone
│
└── README.md              # Ce fichier
```

## Architecture technique

### Serveur
- Utilise **ASIO** (Asynchronous I/O) pour la gestion asynchrone des connexions TCP
- Gère plusieurs clients simultanément avec des pointeurs intelligents (`std::shared_ptr`)
- Protocole texte simple basé sur des commandes préfixées par `#`

### Client
- Interface graphique développée avec **Qt**
- Communication réseau via `QTcpSocket`
- Architecture basée sur les signaux/slots de Qt pour la réactivité de l'interface

### Protocole de communication

Le serveur et le client communiquent via des messages texte terminés par `\n` :

| Message serveur | Description |
|-----------------|-------------|
| `#alias <pseudo>` | Confirmation du pseudo |
| `#connected <pseudo>` | Un utilisateur s'est connecté |
| `#disconnected <pseudo>` | Un utilisateur s'est déconnecté |
| `#renamed <ancien> <nouveau>` | Un utilisateur a changé de pseudo |
| `#list <pseudo1> <pseudo2> ...` | Liste des utilisateurs |
| `#private <pseudo> <message>` | Message privé reçu |
| `#error <code>` | Message d'erreur |

## Dépannage

### Le client ne se connecte pas
- Vérifiez que le serveur est bien lancé
- Vérifiez que le port est correct (par défaut : 3101)
- Vérifiez qu'aucun pare-feu ne bloque la connexion

### Erreur de compilation du serveur
- Assurez-vous d'avoir g++ avec support C++14
- Vérifiez que le chemin vers ASIO est correct

### Erreur de compilation du client
- Vérifiez que Qt est correctement installé
- Vérifiez que les modules `network` et `widgets` sont disponibles
