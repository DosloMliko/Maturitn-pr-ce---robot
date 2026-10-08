#include <esp_now.h>
#include <WiFi.h>

// ── MAC adresa robota ──────────────────────────────────────
// !! ZMĚŇTE NA SKUTEČNOU MAC ADRESU VAŠEHO ROBOTA !!
uint8_t robotMAC[] = { 0x0c, 0x4e, 0xa0, 0x66, 0x65, 0x10 };

// ── Piny joysticku ─────────────────────────────────────────
#define PIN_VRX     34
#define PIN_VRY     35
#define PIN_BTN     32

// ── Kalibrace středu joysticku ─────────────────────────────
// Hodnoty změřené při startu (ruce pryč od joysticku)
int centerX = 2048;
int centerY = 2048;

// Mrtvá zóna – hodnoty uvnitř ignorujeme (eliminace šumu)
#define DEAD_ZONE   80     // ze 4096

// ── Debounce tlačítka ──────────────────────────────────────
#define DEBOUNCE_MS 50
bool    lastBtnState   = HIGH;
bool    btnState       = false;
uint32_t lastDebounce  = 0;

// ── Struktura paketu ───────────────────────────────────────
typedef struct {
    int8_t  x;
    int8_t  y;
    bool    button;
} ControlPacket;

ControlPacket txData;

// ── ESP-NOW callback (potvrzení odeslání) ──────────────────
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Volitelné: Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

// ── Mapování ADC hodnoty na -100 až +100 ──────────────────
int8_t mapAxis(int raw, int center) {
    int delta = raw - center;

    // Mrtvá zóna
    if (abs(delta) < DEAD_ZONE) return 0;

    // Znaménko a normalizace
    if (delta > 0) {
        return (int8_t)map(delta, DEAD_ZONE, center, 0, 100);
    } else {
        return (int8_t)map(delta, -DEAD_ZONE, -center, 0, -100);
    }
}

// ── Setup ──────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    pinMode(PIN_BTN, INPUT_PULLUP);

    // Kalibrace středu joysticku při startu
    // (průměr 16 vzorků pro stabilitu)
    long sumX = 0, sumY = 0;
    for (int i = 0; i < 16; i++) {
        sumX += analogRead(PIN_VRX);
        sumY += analogRead(PIN_VRY);
        delay(5);
    }
    centerX = sumX / 16;
    centerY = sumY / 16;
    Serial.printf("Kalibrace: centerX=%d, centerY=%d\n", centerX, centerY);

    // ESP-NOW init
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init selhalo!");
        return;
    }
    esp_now_register_send_cb(onDataSent);

    // Registrace peera (robota)
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, robotMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Registrace peera selhala!");
        return;
    }

    Serial.println("Ovladač připraven.");
    Serial.print("Moje MAC: ");
    Serial.println(WiFi.macAddress());
}

// ── Loop ───────────────────────────────────────────────────
void loop() {
    // Čtení os joysticku
    int rawX = analogRead(PIN_VRX);
    int rawY = analogRead(PIN_VRY);

    // Mapování na -100 až +100
    // Osa Y: dopředu = kladná hodnota → záleží na orientaci joysticku,
    // případně invertujte znaménko: txData.y = -mapAxis(rawY, centerY);
    txData.x = mapAxis(rawX, centerX);
    txData.y = -mapAxis(rawY, centerY);

    // Debounce tlačítka
    bool reading = (digitalRead(PIN_BTN) == LOW);   // INPUT_PULLUP → LOW = stisknuto
    if (reading != lastBtnState) {
        lastDebounce = millis();
    }
    if ((millis() - lastDebounce) > DEBOUNCE_MS) {
        btnState = reading;
    }
    lastBtnState = reading;

    txData.button = btnState;

    // Odeslání paketu robotovi
    esp_now_send(robotMAC, (uint8_t *)&txData, sizeof(txData));

    // Interval odeslání ~50 Hz (20 ms)
    delay(20);
}