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
  - `ModelContextProtocol` + `AllToolsets` (inclus avec le moteur **5.8**, activés) — **Unreal MCP officiel** (expérimental Epic) : serveur HTTP `127.0.0.1:8000/mcp`, ~52 toolsets exposés via `opencode.json` (serveur remote `unreal-mcp`). Outils de référence : `editor_toolset.toolsets.scene.SceneTools` (placement/retrait d'acteurs, outliner, caméra), `editor_toolset.toolsets.actor.ActorTools`, `editor_toolset.toolsets.asset.AssetTools`, `editor_toolset.toolsets.object.ObjectTools`, `editor_toolset.toolsets.blueprint.BlueprintTools`, `editor_toolset.toolsets.material.*`. Settings dans `Config/DefaultEditorPerProjectUserSettings.ini` (section `[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings]` : port 8000, `bAutoStartServer=True`, `bEnableToolSearch=True`).
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
  QiXX/                      CODE DU JEU — éléments généraux et réutilisables (présents dans tous les niveaux)
    Core/                    Éléments core (GameMode, Controller, Character, input, curseur)
      BP_QiXXGameMode / BP_QiXXController / BP_QiXXCharacter
      Input/  IMC_Default, IA_SetDestination_Click, IA_SetDestination_Touch
      Cursor/ FX_Cursor*, M_Cursor, MI_Cursor_Red, SM_CursorMesh*, T_Arrow
    Characters/Hero/         Héros réutilisable dans chaque niveau : BP_Player, ABP_Player
    Characters/Enemies/      Comportement des ennemis communs (qu'on retrouve un peu partout) : BP_Enemy, BP_EnemyAI, BP_EnemyBeholder, BP_EnemySpawner, ABP_Enemy
    Assets/                  Assets généraux partagés (ex. MI_Colorway)
    UI/                      Menus généraux / HUD réutilisables : WBP_HUD, WBP_Loading, WBP_GameOver
    Localization/            String tables de localisation (ST_UI)
  Level01/                   NIVEAU 01 — tout ce qui est SPÉCIFIQUE à ce niveau (miroir de l'arborescence QiXX)
    Lvl_01.umap              Carte par défaut (startup + gameplay)
    PA_Level01               PrimaryAssetLabel du niveau (ChunkId 1001, packaging)
    Assets/                  Assets spécifiques au niveau 01
    Characters/Enemies/      Ennemis UNIQUES à ce niveau (comme QiXX/Characters/Enemies)
    UI/                      UI spécifiques au niveau 01
  MonsterForSurvivalGame/ RPGTinyFantasyForest/  Paquets de contenu (Megascans/content packs) — RÉFÉRENCES seulement, ne doivent pas faire partie du build
  Collections/ Developers/   Dossiers vides/utilitaires de l'éditeur — ne pas considérer comme du contenu de jeu
  __ExternalActors__/ __ExternalObjects__/  Générés par World Partition — ne pas éditer à la main
```

**Règle d'organisation** : `Content/QiXX/` = **générique/réutilisable** (parties communes du jeu, présentes dans chaque niveau). Un dossier `LevelNN/` (ex. `Level01/`) = tout ce qui est **spécifique à ce niveau** (assets, ennemis uniques, UI propres au niveau). **L'arborescence d'un `LevelNN/` reprend celle de `QiXX/`** (Assets, Characters/Enemies, UI, etc.) pour rester cohérent. Quand un nouvel élément est ajouté, se demander : réutilisable partout → `QiXX/…` ; propre à un niveau → `LevelNN/…`. Les paquets de contenu Megascans/packs (MonsterForSurvivalGame, RPGTinyFantasyForest) servent de référence mais ne doivent pas devenir des références obligatoires du build.

### Packaging par niveaux téléchargeables (chunks)

Chaque niveau est un **chunk distinct téléchargeable séparément** (le joueur installe le jeu de base + seulement les niveaux voulus) :
- Chunk **0** = base (moteur + `QiXX/` partagé). Chunk **1001** = Level01, **1002** = Level02, etc.
- `Config/DefaultGame.ini` :
  - `[/Script/Engine.AssetManagerSettings]` : le type primaire `Map` scanne `Directories=((Path="/Game/Level01"))` (ajouter `"/Game/LevelNN"` pour chaque niveau) ; `bShouldAcquireMissingChunksOnLoad=True` (acquisition des chunks manquants au runtime).
  - `[/Script/UnrealEd.ProjectPackagingSettings]` : `bGenerateChunks=True`.
- Chaque niveau a un **`PrimaryAssetLabel`** (`Content/LevelNN/PA_LevelNN`) avec `bLabelAssetsInMyDirectory=True` (labelise tout le dossier du niveau récursivement) et `rules.chunkId=<1000+NN>`, `cookRule=AlwaysCook`, `bApplyRecursively=False` (le contenu partagé `QiXX/` reste en chunk 0, pas dupliqué dans chaque niveau).
- **Procédure pour créer un `LevelNN/`** : 1) créer les dossiers (Assets, Characters/Enemies, UI) comme Level01 ; 2) créer `PA_LevelNN` (PrimaryAssetLabel) avec le `ChunkId` suivant ; 3) ajouter `"/Game/LevelNN"` à la liste des dossiers scannés du type `Map` dans `DefaultGame.ini`.

⚠️ **`Lvl_01` est en World Partition** (external actors) : les acteurs persistés vivent dans `Content/__ExternalActors__/Level01/Lvl_01/<hex>/<hex>/<GUID>.uasset`, **pas** dans `Lvl_01.umap`. Conséquences :
- Placer un acteur (ex. via `SceneTools.add_to_scene_from_asset`) crée un external actor séparé ; le `.umap` n'est pas modifié.
- `AssetTools.save_assets` sur `/Game/Level01/Lvl_01` ne sauvegarde **pas** les acteurs ajoutés (le package world n'est pas marqué dirty).
- Pour persister : `save_actor` (marche si l'acteur a un package external actor), ou Ctrl+S dans l'éditeur (écrit le `.uasset` external actor dans `__ExternalActors__`).
- Ne pas commiter `Lvl_01.umap` seul ; commit aussi les `.uasset` nouveaux/modifiés sous `Content/__ExternalActors__/Level01/Lvl_01/`.

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
2. **Test fonctionnel in-éditeur (PIE)** : deux voies MCP coexistantes (configurées dans `opencode.json`) :
   - **BlueprintMCP** (community, graphes BP/validation/snapshots) : `node Plugins/BlueprintMCP/Tools/dist/index.js`, port **9847**.
     - `UE_PROJECT_DIR` doit être un **chemin ABSOLU** (`C:/Codes/QiXX`) — un chemin relatif fait échouer le spawn du commandlet.
     - Si l'éditeur UE est ouvert : mode éditeur (port 9847 exposé). Sinon le serveur spawn `UnrealEditor-Cmd.exe -run=BlueprintMCP` (headless, ~10-15 s de démarrage, log dans `Saved/Logs/BlueprintMCP_server.log`).
     - **Piège mode éditeur** : juste après le démarrage de l'éditeur, l'index peut être vide (scan parti avant la fin du chargement des assets). Dans ce cas `list_blueprints` ne montre que des assets moteur → déclencher un rescan : `POST http://localhost:9847/api/rescan` (ou outil `rescan_assets`).
     - Outils de contrôle : `server_status`, `list_blueprints`, `get_blueprint`, `get_blueprint_graph`, `search_blueprints`, `describe_graph`, `find_asset_references`, `search_by_type`, `shutdown_server` (cf. `Plugins/BlueprintMCP/CLAUDE.md`).
   - **Unreal MCP officiel** (placement d'acteurs, assets, outliner, caméra, propriétés) : serveur intégré éditeur, HTTP `http://127.0.0.1:8000/mcp` (remote `unreal-mcp` dans OpenCode).
     - Méta-tools top-level : `list_toolsets`, `describe_toolset`, `call_tool`.
     - **Piège naming** : `describe_toolset`/`call_tool` exigent le **nom interne complet** du toolset (ex. `editor_toolset.toolsets.scene.SceneTools`), et `call_tool` prend `toolset_name` + `tool_name` **sans** préfixe (ex. `tool_name="get_current_level"`).
     - **Piège `find_actors`** : ses params `tag` et `collision_channels` sont required par le schéma — passer des valeurs vides (`""`, `[]`).
     - **Limite persistance** : `add_to_scene_from_asset` ne marque pas le package dirty → `save_assets` retourne `true` sans écrire. Voir la note World Partition plus haut (persister via `save_actor` ou Ctrl+S). Toolset `SceneTools` : `get_current_level`, `find_actors`, `add_to_scene_from_asset`, `add_to_scene_from_class`, `remove_from_scene`, `save_actor`, `set_actor_folder`, `delete_folder`…
3. **Vérification** : après une modification d'asset/Blueprint, re-run du build si C++ modifié, sinon validation directe dans PIE via MCP (play/stop, lecture des states).
4. **Nettoyage** : arrêter le serveur MCP BlueprintMCP avec `shutdown_server` (ou tuer le process) ; vérifier qu'aucun `UnrealEditor-Cmd.exe` ne traîne et que le port 9847 est libéré. Le serveur officiel vit dans l'éditeur (s'arrête avec lui).
5. **OpenCode** : la config (`opencode.json`) se charge au démarrage — après une modif de config, **redémarrer OpenCode**.
