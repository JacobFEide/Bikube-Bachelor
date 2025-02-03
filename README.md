# Bikube-Bachelor
Relevante filer til vårt bachelorprosjekt.
Løsningen vi landet på er en ESP-enhet med et SIM-kort fra Telenor og et tilkoblet modem kobler seg til det lokale 4G-nettet for å opprette en stabil internettforbindelse.
Deretter sender ESP-en data via MQTT-protokollen til en sikker og robust MQTT-server. Denne serveren fungerer som en mellomstasjon for pålitelig datakommunikasjon.
MQTT-serveren videresender data til en database for lagring og videre behandling. 
Til slutt hentes data fra databasen og sendes til OpenHAB, hvor de kan brukes til visualisering av tilkoblede enheter.
