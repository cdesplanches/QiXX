---
name: add-actor-to-level
description: Place, spawn, add, or inject a blueprint/actor instance into a level (e.g. put BP_Hello in Lvl_01) when no editor placement tool exists. Use when the user asks to add/put/spawn/place an actor or blueprint into a map so it appears when the level plays.
---

# Add Actor to Level (workaround)

Le plugin BlueprintMCP n'a **pas** d'outil natif pour placer une instance d'acteur
dans un level. Contournement : injecter un nœud `SpawnActorFromClass` dans le
**Level Blueprint** de la map cible, branché sur `Event BeginPlay`. L'acteur est
spawné au lancement de la map (visible en PIE, pas dans la vue éditeur).

## Prérequis

- Serveur MCP actif : `blueprint-mcp_server_status` doit répondre.
- Si l'index est vide (juste après démarrage éditeur), rescan :
  `POST http://localhost:9847/api/rescan` (ou outil `rescan_assets`).
- Vérifier que la classe à spawner existe : `list_blueprints(filter="<Nom>")`.

## Procédure

1. **Identifier la map cible** : `list_blueprints(type="level")` → noter le nom
   exact du Level Blueprint (ex. `Lvl_01` pour `/Game/Level01/Lvl_01`).
2. **Lire l'EventGraph du Level Blueprint** :
   `get_blueprint_graph(blueprint="<Map>", graph="EventGraph")`
   → noter l'id du nœud `Event BeginPlay` (eventName `ReceiveBeginPlay`) et de son
   pin exec `then`.
3. **Ajouter le nœud de spawn** :
   `add_node(blueprint="<Map>", graph="EventGraph", nodeType="SpawnActorFromClass",
   actorClass="<NomDuBP>")` → noter l'id retourné et la liste des pins (noms exacts).
4. **Câbler BeginPlay → Spawn** :
   `connect_pins(blueprint="<Map>", sourceNodeId="<BeginPlayId>", sourcePinName="then",
   targetNodeId="<SpawnNodeId>", targetPinName="execute")`.
5. **Régler la transformation de spawn** (optionnel) :
   `set_pin_default(blueprint="<Map>", nodeId="<SpawnNodeId>", pinName="Spawn Transform",
   value="Location=(X=0.0,Y=0.0,Z=0.0) Rotation=(Pitch=0.0,Yaw=0.0,Roll=0.0) Scale=(X=1.0,Y=1.0,Z=1.0)")`.
   Adapter le nom de pin et le format au retour de l'étape 3 si différent.
6. **Compiler** : `validate_blueprint(blueprint="<Map>")` → doit retourner `Valid: true`.
   Compile + sauvegarde le package .umap.
7. **Informer l'utilisateur** : jouer la map (Play) pour voir l'acteur apparaître.
   Rappeler la limite du workaround : pas d'instance visible dans l'éditeur,
   l'acteur est spawné à l'exécution.

## Notes

- Ne pas modifier le plugin BlueprintMCP (pas d'outil natif implémenté).
- Cette manipulation **modifie la map** (package .umap sauvé à l'étape 6) — le
  signaler à l'utilisateur (à commiter séparément si besoin).
- Si plusieurs `Event BeginPlay` existent, brancher sur le premier retourné.
- Tester la class au préalable avec `validate_blueprint(blueprint="<NomDuBP>")`.
