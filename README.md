# 🤖 Malý jezdící robot

Jednoduchý dálkově ovládaný robot postavený na **ESP32**, který přijímá ovládací příkazy přes **ESP-NOW**. Ovládání zajišťuje joystick připojený k druhému ESP32.

> **Poznámka:** V aktuálně nahraných souborech `Maly_jezdici_robot.ino` a `ovladac.ino` je stejný kód vysílače/ovladače. Kód přijímače v robotovi a řízení motorů není v těchto souborech obsažen.

## ✨ Funkce

- 🎮 ovládání pomocí analogového joysticku
- 📡 bezdrátová komunikace přes ESP-NOW
- ⚡ odesílání příkazů přibližně **50× za sekundu**
- 🎯 automatická kalibrace středu joysticku při spuštění
- 🛑 mrtvá zóna joysticku pro potlačení šumu
- 🔘 tlačítko na joysticku s debounce ochranou
- 📊 hodnoty os joysticku jsou převedeny na rozsah `-100 až 100`

## 🧰 Hardware

### Ovladač

- ESP32
- analogový joystick
- připojení joysticku:

| Joystick | ESP32 |
|---|---:|
| VRX | GPIO 34 |
| VRY | GPIO 35 |
| SW / BTN | GPIO 32 |
| GND | GND |
| VCC | 3.3V |

### Robot

Robot potřebuje druhé ESP32, které přijímá data přes ESP-NOW a podle nich ovládá motory.

## 📡 Komunikace

Ovladač používá **ESP-NOW**, takže není potřeba Wi-Fi router ani internet.

Odesílaný paket má tuto strukturu:

```cpp
typedef struct {
    int8_t x;
    int8_t y;
    bool button;
} ControlPacket;
```

- `x` – pohyb joysticku v ose X (`-100 až 100`)
- `y` – pohyb joysticku v ose Y (`-100 až 100`)
- `button` – stav tlačítka (`true` / `false`)

Data se odesílají každých přibližně **20 ms**, tedy asi **50 Hz**.

## 🎯 Kalibrace joysticku

Po zapnutí se automaticky změří střed joysticku.

Ovladač během kalibrace provede 16 měření a z jejich průměru určí hodnoty:

```cpp
centerX
centerY
```

Proto je důležité při zapnutí **nechat joystick v klidu a nedotýkat se ho**.

Pro omezení drobných nepřesností je použita mrtvá zóna:

```cpp
#define DEAD_ZONE 80
```

## 🔧 Nastavení MAC adresy

Ovladač musí znát MAC adresu ESP32 v robotovi.

V kódu se nachází:

```cpp
uint8_t robotMAC[] = {
    0x0c, 0x4e, 0xa0, 0x66, 0x65, 0x10
};
```

Tuto hodnotu je potřeba změnit na MAC adresu přijímacího ESP32.

MAC adresu ovladače lze zobrazit v Serial Monitoru po spuštění:

```text
Ovladač připraven.
Moje MAC: XX:XX:XX:XX:XX:XX
```

## 🚀 Nahrání programu

1. Nainstaluj **Arduino IDE**.
2. Přidej podporu pro **ESP32**.
3. Připoj ESP32 ovladače přes USB.
4. Otevři `ovladac.ino`.
5. Nastav správnou desku a COM port.
6. Zkontroluj MAC adresu robota v programu.
7. Nahraj program do ESP32.
8. Otevři Serial Monitor na rychlosti **115200 baud**.
9. Po startu nech joystick několik okamžiků v klidu kvůli kalibraci.

## 🖥️ Serial Monitor

Po úspěšném spuštění by se mělo zobrazit například:

```text
Kalibrace: centerX=2048, centerY=2051
Ovladač připraven.
Moje MAC: XX:XX:XX:XX:XX:XX
```

Pokud se nepodaří inicializovat ESP-NOW, program vypíše:

```text
ESP-NOW init selhalo!
```

## 📁 Struktura projektu

```text
.
├── Maly_jezdici_robot.ino
├── ovladac.ino
└── README.md
```

> Aktuálně oba `.ino` soubory obsahují stejný program ovladače. Doporučuje se později oddělit kód vysílače a přijímače do samostatných souborů.

## 🛠️ Použité technologie

- **ESP32**
- **Arduino**
- **ESP-NOW**
- **Wi-Fi STA mode**
- analogový joystick

## 📌 Plánované vylepšení

- [ ] doplnit program přijímače na robotovi
- [ ] přidat řízení motorů
- [ ] přidat bezpečnostní timeout při ztrátě spojení
- [ ] zobrazovat stav spojení
- [ ] přidat nastavení rychlosti
- [ ] vylepšit kalibraci joysticku
- [ ] vytvořit schéma zapojení
- [ ] přidat fotografie robota

## 📄 Licence

Projekt můžeš používat a upravovat podle potřeby. Pokud chceš projekt zveřejnit pod konkrétní licencí, doporučuji například **MIT License**.
