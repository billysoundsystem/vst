# OMBRA — Synthé wavetable (VST3 + Standalone)

Synthétiseur instrument fait avec JUCE 8.
Moteur : oscillateur wavetable morphable → filtre passe-bas résonant → enveloppe ADSR,
LFO sur le cutoff, polyphonie 8 voix. Chaîne FX : Drive → Chorus → Delay → Reverb → Pan.
Clavier MIDI intégré, interface sombre.

---

## Ce dont tu as besoin (une seule installation)

**Visual Studio 2022 Community** (gratuit), avec le module coché à l'installation :
**« Développement Desktop en C++ »** — il inclut CMake.

Tu n'as PAS besoin de télécharger JUCE : le projet le récupère tout seul au premier build.

---

## Compiler

Ouvre une invite de commandes (cmd) dans ce dossier, puis :

```
cmake -B build
cmake --build build --config Release
```

- Le premier `cmake -B build` télécharge JUCE automatiquement (quelques minutes,
  connexion internet requise). Les fois suivantes c'est instantané.
- Compile bien en **Release** (perfs + taille réduite).

### Alternative tout-en-clic
Tu peux aussi ouvrir directement ce dossier dans Visual Studio 2022
(*Fichier → Ouvrir → Dossier*). VS détecte le CMake et propose de configurer/compiler
depuis le menu, en choisissant la configuration **Release**.

---

## Récupérer le plugin

Après compilation :

- **VST3** : `build/Ombra_artefacts/Release/VST3/Ombra.vst3`
  → copie-le dans `C:\Program Files\Common Files\VST3\`
  → dans FL Studio : *Options → Manage plugins → Find more plugins* (rescan).

- **Standalone** (pour tester sans DAW) :
  `build/Ombra_artefacts/Release/Standalone/Ombra.exe`
  (pense à choisir ta carte son / le port MIDI dans *Options* de l'appli).

---

## Renommer le plugin

Dans `CMakeLists.txt`, modifie :
- `PRODUCT_NAME "Ombra"` → le nom affiché
- `COMPANY_NAME "Billysoundsystem"`
- `PLUGIN_CODE Omb1` → garde 4 caractères, dont au moins une majuscule, unique par plugin

Puis recompile.

---

## Les paramètres

- **OSC** : Wave (morphing sinus → triangle → saw → square)
- **FILTER** : Cutoff, Reso, Env (montant de l'enveloppe sur le cutoff)
- **ENVELOPPE** : A / D / S / R
- **LFO** : Rate, Depth (module le cutoff)
- **FX** : Drive, Chorus, Delay (Time / Fbk / Mix), Reverb (Size / Mix), Pan
- **MASTER** : Volume

---

## Notes

- Un splash JUCE peut s'afficher brièvement (normal en usage perso / licence GPL de JUCE).
- Compilé et vérifié sans erreur (VST3 + Standalone). Les réglages de son et l'UI
  pourront être ajustés après tes premiers tests dans FL Studio — c'est itératif.
