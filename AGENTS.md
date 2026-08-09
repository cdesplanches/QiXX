# AGENTS.md — QiXX (Unreal Engine 5.8)

Guide de travail pour les agents IA (OpenCode) sur le projet QiXX.

## Vue d'ensemble

- **Moteur** : Unreal Engine **5.8** uniquement (`C:\Program Files\Epic Games\UE_5.8`). Ne pas cibler une autre version.
- **Projet** : `C:\Codes\QiXX\QiXX.uproject`. Module runtime `QiXX` (`Source/QiXX`).
- **Style** : jeu **isométrique** (Top-Down 2.5D). Seule la variante isométrique est conservée.
- **Plugins** :
  - `BlueprintMCP` (sous-dossier de `Plugins/`) — serveur MCP pour piloter l'éditeur/le contenu (outils, indexation, snapshots). **Copie trackée dans le projet** (pas de repo interne).
  - `Autonomix` — assistant IA in-éditeur (LLM, exécution d'actions). **Copie trackée dans le projet** (pas de repo interne).
  - `VisualStudioTools` (inclus avec le moteur, activé) — test explorer. Tracké en **fichiers normaux**.
  - `ModelingToolsEditorMode` (inclus, activé).
- **Logs** : catégorie `LogQiXX` (`QiXX.h`). Toujours logger via `UE_LOG(LogQiXX, ...)`.

## Architecture

```
Source/QiXX/
  QiXX.cpp / QiXX.h          Module runtime, catégorie LogQiXX
  Isometric/                 Classes de base C++ (logique de jeu isométrique)
    IsometricGameMode.h/.cpp           AIsometricGameMode
    IsometricCharacter.h/.cpp          AIsometricCharacter
    IsometricPlayerController.h/.cpp   AIsometricPlayerController

Content/
  QiXX/Core/                 Blueprints parents des classes C++ + input/cursor
    BP_QiXXGameMode / BP_QiXXController / BP_QiXXCharacter
    Input/  IMC_Default, IA_SetDestination_Click, IA_SetDestination_Touch
    Cursor/ FX_Cursor*, M_Cursor, MI_Cursor_Red, SM_CursorMesh*, T_Arrow
  QiXX/Characters/Hero/      BP_Player, ABP_Player
  QiXX/Characters/Enemies/   BP_Enemy, BP_EnemyAI, BP_EnemyBeholder, BP_EnemySpawner, ABP_Enemy
  QiXX/UI/                   WBP_HUD, WBP_Loading, WBP_GameOver
  QiXX/Assets/               MI_Colorway
  QiXX/Localization/         ST_UI
  Blueprints/                Blueprints utilitaires (BP_EnemyActor, BP_EnemySpawner_C)
  Level01/Lvl_01             Carte par défaut (startup + gameplay)
  MonsterForSurvivalGame/ RPGTinyFantasyForest/   Assets Megascans/content packs
```

Classes C++ (Base) → Blueprints (Parents) :
- `AIsometricGameMode` → `BP_QiXXGameMode` (`Content/QiXX/Core/`), défini comme `GlobalDefaultGameMode` dans `DefaultEngine.ini`.
- `AIsometricPlayerController` → `BP_QiXXController` (Enhanced Input : `IMC_Default` + actions `IA_*`).
- `AIsometricCharacter` → `BP_QiXXCharacter` (caméra isométrique via `CameraBoom` + `IsometricCamera`, pitch -45°, yaw 45°, `Health`, `WalkSpeed`).

Autres BPs de gameplay (Blueprints purs, pas de classe C++ dédiée) :
- Hero : `BP_Player` (+ `ABP_Player`).
- Ennemis : `BP_Enemy`, `BP_EnemyBeholder` (enfant de `BP_Enemy`), `BP_EnemyAI` (AIController), `BP_EnemySpawner` (+ `ABP_Enemy`).
- UI : `WBP_HUD`, `WBP_Loading`, `WBP_GameOver`.

### Config clé (`Config/DefaultEngine.ini`)
- `GameDefaultMap` / `EditorStartupMap` : `/Game/Level01/Lvl_01`.
- `GlobalDefaultGameMode` : `/Game/QiXX/Core/BP_QiXXGameMode`.
- Navigation : `RecastNavMesh`, `RuntimeGeneration=Dynamic`, `bForceRebuildOnLoad=True`.
- Rendering : Ray Tracing activé, Substrate, Virtual Shadow Maps, DX12 (SM6).
- Redirections héritées de `TP_TopDown` (`/Script/QiXX`).

## Conventions C++

- **C++ moderne compatible UE** : `TObjectPtr<T>` pour les UPROPERTY de références, `CreateDefaultSubobject`, `GENERATED_BODY()`.
- **Dépendances module** (`QiXX.Build.cs`) : `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`. Ajouter un module dans le .Build.cs si un nouveau header est inclus.
- **Headers** : style classique UE (un `.h` + `.cpp` par classe), `#pragma once`, include du `.generated.h` en dernier dans le header, includes système/engine avant les headers du projet.
- **Naming des identifiants** : classes `A`/`U`/`F`/`E`/`I` (prefixes UE standards), méthodes `PascalCase`, variables `PascalCase`, paramètres/membres privés suivent la convention UE par défaut.
- **Accesseurs membres** : privilégier `GetX()` / `SetX()` ; ne pas exposer les membres privés.
- **Logging** : `UE_LOG(LogQiXX, Log/Warning/Error, TEXT(...))`.
- **Ne PAS mettre de commentaires dans le code sauf demande explicite.**

## Conventions Blueprint

- **Préfixes** : `BP_` pour les Blueprints (ex. `BP_QiXXGameMode`, `BP_EnemyActor`).
- Les Blueprints « Core » parentent les classes C++ (`Content/QiXX/Core/`) — la logique de gameplay doit rester en C++ quand elle est critique, la personnalisation/assemblage en Blueprint.
- Naming des assets : `Content/` organisé par fonction (Characters, UI, Assets, Core, Localization).
- Préférer les assets **isométriques/neutres** ; les paquets de contenu prototypes (MonsterForSurvivalGame, RPGTinyFantasyForest) restent mais ne doivent pas devenir des références obligatoires.

## Multiplayer & Replication

- L'architecture actuelle est **mono-joueur** (pas de logique réseau implémentée).
- Règles à respecter si du multijoueur est ajouté :
  - La simulation/autorité réseau vit **côté serveur** (`Authority`), les clients ne font que prédire/présenter.
  - Utiliser `Server`/`Client`/`Multicast` RPC (UFUNCTION) et les variables `Replicated`/`ReplicatedUsing` pour toute donnée qui doit être synchro.
  - Camera et input (WASD) côté client uniquement ; mouvement via `AddMovementInput` sur le serveur.
  - **Ne jamais** placer de logique de gameplay critique dans des fonctions Blueprint non répliquées.

## Naming (rappel transversal)

| Élément | Règle | Exemple |
|---|---|---|
| Classes C++ | Préfixe UE | `AIsometricGameMode` |
| Blueprints | `BP_` | `BP_QiXXGameMode` |
| Maps | `Lvl_` | `Lvl_01` |
| Assets | minuscules snake_case (contenu) | `bp_enemyactor` |
| Config | PascalCase | `DefaultEngine.ini` |

## Règles Git

- **Ne jamais commiter sans demande explicite de l'utilisateur.**
- `Binaries/`, `Intermediate/`, `DerivedDataCache/`, `Saved/`, `*.log`, `*.dll`, `*.pdb`, `*.exe` sont **ignorés** (`Build/` et `Saved/` du projet sont régénérés par les builds).
- `Plugins/BlueprintMCP`, `Plugins/Autonomix` et `Plugins/VisualStudioTools` sont trackés en **fichiers normaux** dans le dépôt (copies autonomes). Décision : le projet QiXX est **autonome** — **aucun** repo interne, **aucun** submodule, **aucune** dépendance distante. Un clone du projet contient tout.
  - Conséquence : pas de `.gitmodules`, pas de pointeurs gitlink. Pour mettre à jour un plugin, remplacer son contenu par la nouvelle version (copie de fichiers), sans `.git` interne.
  - Serveur MCP : s'exécute depuis `Plugins/BlueprintMCP/Tools/dist/` (compilé, tracké). `Tools/node_modules/` est ignoré — `npm install` une fois dans `Plugins/BlueprintMCP/Tools/` s'il est absent. `Plugins/Autonomix/Config/` est ignoré (clés API utilisateur — ne jamais commiter).
- Messages de commit : courts, style de l'historique existant (impératif concis, ex. « Adjust Enemy position + capsule collider »), sur une ligne.
- Vérifier `git status` + `git diff` avant tout commit ; ne stager que les fichiers voulus ; ne jamais commiter de secrets.
- Le fichier `Binaries/Win64/UnrealEditor.modules` est **régénéré** par le build et ignoré — ne pas le commiter.

## Règles de compilation (UE 5.8)

Lancer le build depuis la racine du projet (PowerShell) :

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" QiXXEditor Win64 Development -project="C:\Codes\QiXX\QiXX.uproject" -WaitMutex
```

- Cible : `QiXXEditor` (éditeur), config `Development`, plateforme `Win64`.
- **Compilation des modules** : toujours après une modif C++, puis redémarrer l'éditeur UE pour charger les nouvelles DLL (pas de hot-reload fiable entre versions).
- En cas d'erreur, corriger les **incompatibilités d'API 5.8** (headers déplacés, `FJsonObject::Values` keyé par `UE::FSharedString`, etc.) plutôt que de contourner avec des macros de compat.
- Le projet se build via `UnrealBuildTool` (dans `Engine\Binaries\DotNET\UnrealBuildTool\`).

## Procédure de test

1. **Compilation** : lancer la commande de build ci-dessus ; viser zéro erreur (les warnings C4996 de dépréciation sont tolérés mais à nettoyer si possible).
2. **Test fonctionnel in-éditeur (PIE)** : les instructions de gameplay s'exécutent via le plugin **BlueprintMCP** (configuré dans `opencode.json`).
   - Serveur MCP : `node Plugins/BlueprintMCP/Tools/dist/index.js`, port **9847**.
   - `UE_PROJECT_DIR` doit être un **chemin ABSOLU** (`C:/Codes/QiXX`) — un chemin relatif fait échouer le spawn du commandlet.
   - Si l'éditeur UE est ouvert : mode éditeur (port 9847 exposé). Sinon le serveur spawn `UnrealEditor-Cmd.exe -run=BlueprintMCP` (headless, ~10-15 s de démarrage, log dans `Saved/Logs/BlueprintMCP_server.log`).
   - **Piège mode éditeur** : juste après le démarrage de l'éditeur, l'index peut être vide (scan parti avant la fin du chargement des assets). Dans ce cas `list_blueprints` ne montre que des assets moteur → déclencher un rescan : `POST http://localhost:9847/api/rescan` (ou outil `rescan_assets`).
   - Outils de contrôle : `server_status`, `list_blueprints`, `get_blueprint`, `get_blueprint_graph`, `search_blueprints`, `describe_graph`, `find_asset_references`, `search_by_type`, `shutdown_server` (cf. `Plugins/BlueprintMCP/CLAUDE.md`).
3. **Vérification** : après une modification d'asset/Blueprint, re-run du build si C++ modifié, sinon validation directe dans PIE via MCP (play/stop, lecture des states).
4. **Nettoyage** : arrêter le serveur MCP avec `shutdown_server` (ou tuer le process) ; vérifier qu'aucun `UnrealEditor-Cmd.exe` ne traîne et que le port 9847 est libéré.
5. **OpenCode** : la config (`opencode.json`) se charge au démarrage — après une modif de config, **redémarrer OpenCode**.
