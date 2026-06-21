# Compiler Ombra en ligne (sans rien installer)

Cette méthode fait compiler le plugin par un **vrai ordinateur Windows dans le cloud**
(gratuit, via GitHub). Tu n'installes ni Visual Studio ni JUCE. Tu récupères juste
le `.vst3` à la fin.

---

## Étapes (≈ 10 min, surtout de l'attente)

### 1. Crée un compte GitHub
Va sur https://github.com → *Sign up* (gratuit). Si tu as déjà un compte, connecte-toi.

### 2. Crée un dépôt (repository)
- Clique sur le **+** en haut à droite → *New repository*.
- Donne-lui un nom, ex. `ombra-synth`.
- Laisse en **Public** (gratuit et illimité pour les builds).
- Clique *Create repository*.

### 3. Envoie les fichiers du projet
Sur la page du dépôt vide :
- Clique sur **« uploading an existing file »** (lien au milieu de la page).
- **Glisse-dépose** tout le contenu du dossier `Ombra` :
  `CMakeLists.txt`, le dossier `Source`, et le dossier `.github`.

  ⚠️ Important : le dossier **`.github`** doit être envoyé (c'est lui qui déclenche
  la compilation). S'il n'apparaît pas dans ton explorateur Windows, active
  *Affichage → Éléments masqués*. Le plus simple : sélectionne tout l'intérieur du
  dossier `Ombra` d'un coup et glisse-le.
- En bas, clique ***Commit changes***.

### 4. La compilation démarre toute seule
- Va dans l'onglet **Actions** (en haut du dépôt).
- Tu verras un build **« Build Windows »** en cours (rond jaune qui tourne).
- Attends qu'il passe au **vert** (quelques minutes — il télécharge JUCE puis compile).
- S'il devient rouge, ouvre-le, copie le message d'erreur et envoie-le-moi.

### 5. Télécharge ton plugin
- Clique sur le build vert terminé.
- En bas, section **Artifacts**, tu as :
  - **Ombra-VST3-Windows** → ton plugin
  - **Ombra-Standalone-Windows** → la version test sans DAW
- Télécharge **Ombra-VST3-Windows** (ça donne un .zip).

### 6. Installe dans FL Studio
- Décompresse le .zip → tu obtiens un dossier **`Ombra.vst3`**.
- Copie ce dossier `Ombra.vst3` dans :
  `C:\Program Files\Common Files\VST3\`
- Dans FL Studio : *Options → Manage plugins → Find more plugins* (rescan).
- Ombra apparaît dans la liste des instruments.

---

## En cas de souci
Si le build passe au rouge, ne touche à rien : copie-moi le texte de l'erreur
(onglet Actions → le build → l'étape rouge) et je corrige.
