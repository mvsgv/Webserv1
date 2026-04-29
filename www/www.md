# La logique derrière le serveur et le fichier HTML

## 1. La vraie nature d'un Serveur Web (Ton Webserv)
Il faut démystifier ce qu'est un serveur web. Ton programme C++ **ne crée pas** de sites web. Ton programme est en réalité un simple "distributeur de fichiers" ou un "bibliothécaire".

Quand un client (le navigateur Chrome, Firefox, etc.) se connecte à ton serveur sur le port 8080, il ne demande pas "montre-moi un site web". Il envoie une requête textuelle qui dit : *"S'il te plaît, lis le fichier qui s'appelle `/index.html` sur ton disque dur, et envoie-moi son contenu texte exact"*.

La logique, c'est que ton serveur a besoin de "matière première" à distribuer. Ce dossier `www/` est la bibliothèque de ton serveur, et `index.html` sera le premier livre que tu vas poser sur l'étagère pour que ton serveur ait quelque chose à renvoyer.

## 2. Le rôle du Navigateur (Le client)
Quand ton serveur aura utilisé la fonction C++ `send()` pour envoyer le texte du fichier `index.html` à travers le réseau, c'est le navigateur web qui prend le relais. 

Le navigateur est un programme de rendu visuel. Il reçoit une longue chaîne de texte brut depuis ton serveur en C++, et il y cherche des "instructions de dessin". Ces instructions, ce sont les "balises" HTML (les mots entre chevrons `< >`).

## 3. La logique de l'anatomie du fichier HTML
Pour que le navigateur "dessine" correctement la page, il faut lui donner une structure logique universelle. Voici pourquoi on utilise des balises précises :

* **Le contrat initial (`<!DOCTYPE html>`) :** La toute première ligne dit au navigateur : "Attention, le texte qui suit respecte les règles modernes de l'affichage Web (HTML5)". Sans cela, de vieux navigateurs pourraient utiliser des règles d'affichage obsolètes (le mode "quirks").
* **La grande boîte (`<html> ... </html>`) :** Encadre tout le document pour indiquer où la page web commence et où elle se termine.
* **La salle des machines (`<head> ... </head>`) :** Tout ce qui s'y trouve est **invisible** sur l'écran. C'est la logique de paramétrage de la page. 
  * On y met par exemple `<title>Mon Super Site</title>`, ce qui changera le nom de l'onglet dans le navigateur. 
  * On y précise aussi l'encodage (ex: `<meta charset="UTF-8">`) pour que les accents (é, à, ç) s'affichent correctement.
* **La vitrine (`<body> ... </body>`) :** C'est le contenu visuel. Tout ce qui est entre ces deux balises s'affichera au centre de l'écran.
  * On y place des titres avec `<h1>Mon Titre</h1>` (le navigateur le dessinera en gros et en gras).
  * On y place du texte normal avec `<p>Mon paragraphe</p>`.

**En résumé :** L'objectif final de ton Webserv (C++) sera d'ouvrir ce fichier texte sur ton ordinateur (`std::ifstream`), de copier ce texte en mémoire, et de l'envoyer dans les tuyaux du réseau avec `send()`. Ton serveur en C++ se moque de savoir si c'est du HTML, du Chinois ou de la poésie : son travail est de l'expédier intact. C'est le navigateur web de l'utilisateur qui se chargera de la magie visuelle !