# Spiritual_LED_Candle
Lucrare de finalizare a studiilor Bancila Emanuel "Spiritual LED Candle - Automatizare inteligenta pentru un stand votiv"

Link repository github: https://github.com/emy953/Spiritual_LED_Candle

Mod de utilizare:
Folosind Arduino IDE se incarca fisierul Master.ino pe Arduino Uno din modulul principal.

Folosind Arduino IDE se incarca fisierul Slave.ino pe Arduino Nano corespunzator fiecarui modul secundar.

Pentru fiecare modul secundar suplimentar prezent intr-un sistem, se schimba definitia "ADDR" din Slave.ino cu o valoare diferita.

Exemplu: #define ADDR 9 : Arduino Nano din acest modul secundar va avea adresa 9, urmatoarele extensii vor avea oricare adresa cuprinsa intre 0 si 127, dar nu adresa 9.
