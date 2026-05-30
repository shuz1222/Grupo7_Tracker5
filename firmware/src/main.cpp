//  Tracker LoRa-APRS - TEC Grupo 7
//  Callsign : TI0TEC7-7
//  Frecuencia: 433.775 MHz (Banda 70cm, radioaficionados CR)

// --- Librerías ---
#include <Arduino.h>          // Framework Arduino para ESP32
#include <Wire.h>             // Comunicación I²C (OLED, AXP2101)
#include <SPI.h>              // Comunicación SPI (LoRa)
#include <HardwareSerial.h>   // Puerto serie adicional (UART1 para GPS)
#include <TinyGPSPlus.h>      // Parseo de datos NMEA del GPS NEO-M8N
#include <LoRa.h>             // Control del módulo LoRa SX1276

#define XPOWERS_CHIP_AXP2101  // Selecciona el modelo AXP2101
#include <XPowersLib.h>       // Gestión del chip de energía

#include <Adafruit_GFX.h>     // Gráficos básicos para OLED
#include <Adafruit_SSD1306.h> // Control del display SSD1306

// --- CONSTANTES DE CONFIGURACIÓN ---

// Identificación de la estación
#define CALLSIGN        "TI0TEC7-7"    // Identificador único en red APRS
#define APRS_SYMBOL     ">"             // Símbolo APRS: ">" = vehículo/coche
#define APRS_SYMBOL_TAB "/"             // Tabla de símbolos: "/" = primaria
#define APRS_COMMENT    "Tracker TEC - Grupo 7"  // Texto informativo

// Parámetros LoRa
#define LORA_FREQ       433775000
#define LORA_SF         10
#define LORA_BW         125E3
#define LORA_CR         5
#define LORA_SYNC_WORD  0x12
#define LORA_TX_POWER   20

// Pines LoRa SX1276
#define LORA_SCK        5
#define LORA_MISO       19
#define LORA_MOSI       27
#define LORA_CS         18
#define LORA_RST        23
#define LORA_DIO0       26

// Pines GPS NEO-M8N
#define GPS_RX          34 // RX del ESP32 conectado a TX del GPS
#define GPS_TX          12 // TX del ESP32 conectado a RX del GPS
#define GPS_BAUD        9600

// Pines I²C (Compartido entre OLED y AXP2101)
#define I2C_SDA         21
#define I2C_SCL         22

// Pantalla OLED SSD1306
#define OLED_WIDTH      128             // Ancho en píxeles
#define OLED_HEIGHT     64              // Alto en píxeles
#define OLED_RESET      -1              // Sin pin de reset (usado por I²C)
#define OLED_ADDRESS    0x3C            // Dirección I²C típica del SSD1306

// Botón físico
#define BTN_PIN         38

// Parámetros de ciclo
#define SLEEP_DURATION      60      // Segundos de deep sleep
#define MAX_TX_RETRIES      3       // Reintentos de transmisión
#define OLED_INTERVAL       3000    // Alternancia de pantallas (ms)
#define DATOS_FINALES_MS    5000    // Tiempo mostrando datos antes de sleep
#define ESCUCHA_IGATES_MS   5000    // Tiempo escuchando iGates tras TX
#define GPS_TIMEOUT_MS  60000   // 60 segundos

// Frecuencia de CPU durante operación (ahorro de energía)
#define CPU_FREQ_MHZ        80      // 80 MHz en lugar de 240 MHz


// --- VARIABLES GLOBALES ---

// Variables en memoria RTC (persisten durante deep sleep)
RTC_DATA_ATTR int  bootCount  = 0;      // Contador de arranques/inicios
RTC_DATA_ATTR int  txCount    = 0;      // Contador de TX exitosos
RTC_DATA_ATTR bool pantallaOn = true;   // Estado de la pantalla OLED

// Objetos de hardware
TinyGPSPlus       gps;                  // Decodificador de tramas NMEA
HardwareSerial    SerialGPS(1);         // Puerto UART1 para GPS
XPowersAXP2101    PMU;                  // Controlador de energía AXP2101
Adafruit_SSD1306  display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// Flags y variables de control
bool          oledDisponible      = false;   // TRUE si se detectó OLED
bool          mostrarPantalla1    = true;    // TRUE = pantalla posición
unsigned long ultimoCambioPantalla = 0;      // Para alternar pantallas
String        ultimoIGate         = "NONE";  // Último iGate escuchado
String        estadoSistema       = "Iniciando...";  // Texto de estado actual

// --- ESTRUCTURA DE DATOS GPS ---
// Almacena toda la información de navegación obtenida del GPS.
struct DatosGPS {
    float latitud;
    float longitud;
    char  hemisferio_lat;
    char  hemisferio_lon;
    float velocidad_knots;
    float rumbo_grados;
    float altitud_metros;
    bool  valido;
};

// Forward declarations
void entrarDeepSleep(bool mostrarDatosFinales = false);

// Instancia global de la estructura de datos
DatosGPS datosActuales = {0, 0, 'N', 'W', 0, 0, 0, false};
String   tramaActual   = "";    // Trama APRS lista para transmitir

// --- RUTINA: BOTÓN FÍSICO ---
// Alterna encendido/apagado de la pantalla OLED.
void IRAM_ATTR manejarBoton() {
    pantallaOn = !pantallaOn;
    if (pantallaOn) {
        display.ssd1306_command(SSD1306_DISPLAYON);
    } else {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
    }
}

// --- RUTINA: PANTALLA 1 — Datos de posición + satélites ---
// Muestra coordenadas, velocidad, altitud y satélites en OLED.
void mostrarDatosPosicion() {
    // Verificar que la pantalla esté disponible y encendida
    if (!oledDisponible || !pantallaOn) return;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(CALLSIGN);
    display.drawLine(0, 10, OLED_WIDTH, 10, SSD1306_WHITE);

    if (datosActuales.valido) {
        // Hay fix - mostrar posición completa
        display.setCursor(0, 14);
        display.printf("Lat: %.4f%c",
            abs(datosActuales.latitud),
            datosActuales.hemisferio_lat);
        display.setCursor(0, 24);
        display.printf("Lon: %.4f%c",
            abs(datosActuales.longitud),
            datosActuales.hemisferio_lon);
        display.setCursor(0, 34);
        display.printf("Vel: %.1f kt", datosActuales.velocidad_knots);
        display.setCursor(0, 44);
        display.printf("Alt: %.0f m", datosActuales.altitud_metros);
        display.setCursor(0, 54);
        display.printf("Sat: %d", gps.satellites.value());
    } else {
        // Sin fix — mostrar satélites visibles mientras espera
        display.setCursor(0, 20);
        display.println("Sin datos GPS");
        display.setCursor(0, 32);
        display.println("Esperando fix...");
        display.setCursor(0, 44);
        display.printf("Sat visibles: %d", gps.satellites.value());
    }
    display.display();
}

//  RUTINA: PANTALLA 2 — Estado del sistema + último iGate
// Muestra estado del tracker, contadores y voltaje en OLED.
void mostrarEstadoSistema() {
    if (!oledDisponible || !pantallaOn) return;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Estado sistema");
    display.drawLine(0, 10, OLED_WIDTH, 10, SSD1306_WHITE);
    display.setCursor(0, 14);
    display.println(estadoSistema);
    display.setCursor(0, 28);
    display.printf("Boot: #%d  TX: #%d", bootCount, txCount);
    display.setCursor(0, 40);
    display.printf("Bat: %.2fV", PMU.getBattVoltage() / 1000.0);
    display.setCursor(0, 52);
    display.printf("iGate: %s", ultimoIGate.c_str());
    display.display();
}

//  RUTINA: ACTUALIZAR PANTALLA
// Alterna automáticamente entre las dos pantallas cada cierto tiempo.
void actualizarPantalla() {
    if (!oledDisponible || !pantallaOn) return;

    unsigned long ahora = millis();
    // Verificar si es momento de cambiar de pantalla
    if (ahora - ultimoCambioPantalla >= OLED_INTERVAL) {
        mostrarPantalla1 = !mostrarPantalla1;
        ultimoCambioPantalla = ahora;
    }
    // Mostrar la pantalla correspondiente según el flag
    if (mostrarPantalla1) {
        mostrarDatosPosicion();
    } else {
        mostrarEstadoSistema();
    }
}

//  RUTINA 1: INICIALIZACIÓN AXP2101
// Inicializa el chip de gestión de energía AXP2101.
//    DC1   (3.3V) → ESP32
//    ALDO2 (3.3V) → LoRa SX1276
//    ALDO3 (3.3V) → GPS NEO-M8N
//    DC5   (3.3V) → OLED SSD1306
void initAXP2101() {
    Wire.begin(I2C_SDA, I2C_SCL);

    // Intenta comunicarse con el AXP2101
    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("[AXP2101] ERROR — no encontrado, reiniciando...");
        delay(1000);
        ESP.restart(); // Reiniciar si no hay respuesta
    }

    PMU.enableDC1();
    PMU.setDC1Voltage(3300);        // ESP32

    PMU.enableALDO2();
    PMU.setALDO2Voltage(3300);      // LoRa SX1276

    PMU.enableALDO3();
    PMU.setALDO3Voltage(3300);      // GPS NEO-M8N

    PMU.enableDC5();
    PMU.setDC5Voltage(3300);        // OLED SSD1306

    // Configurar carga de batería
    PMU.setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_700MA);  // 700mA
    PMU.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);  // 4.2V

    // Reportar estado
    Serial.printf("[AXP2101] OK — Bat: %.2fV  %d%%\n",
        PMU.getBattVoltage() / 1000.0,
        PMU.getBatteryPercent());
}

//  RUTINA 2: INICIALIZACIÓN OLED
//  Inicializa la pantalla OLED SSD1306.
//  En caso de que la pantalla no responda, se continúa sin ella
void initOLED() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("[OLED] ERROR — pantalla no encontrada");
        oledDisponible = false;
        return;
    }

    oledDisponible = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 10);
    display.println("TI0TEC7-7");
    display.setCursor(10, 25);
    display.println("Tracker LoRa-APRS");
    display.setCursor(15, 40);
    display.println("Iniciando...");
    display.display();

    if (!pantallaOn) {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
    }

    Serial.println("[OLED] OK — pantalla inicializada");
}

//  RUTINA 3: INICIALIZACIÓN BOTÓN
//  Configura el botón físico GPIO38 como entrada con interrupción.
void initBoton() {
    pinMode(BTN_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BTN_PIN),
                    manejarBoton,
                    FALLING);
    Serial.println("[BTN] OK — GPIO38 configurado");
}

//  RUTINA 4: INICIALIZACIÓN GPS
// Inicializa el puerto serie UART1 para comunicación con el GPS.
void initGPS() {
    SerialGPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
    delay(100);
    Serial.println("[GPS] UART1 OK — 9600 bps");
}

//  RUTINA 5: INICIALIZACIÓN LORA SX1276
// Inicializa el módulo LoRa SX1276 con los parámetros APRS.
void initLoRa() {
    // Inicializar bus SPI con los pines configurados
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

    // Intentar iniciar LoRa en la frecuencia configurada
    if (!LoRa.begin(LORA_FREQ)) {
        Serial.println("[LoRa] ERROR — SX1276 no responde, reiniciando...");
        if (oledDisponible) {
            display.clearDisplay();
            display.setCursor(0, 20);
            display.println("LoRa ERROR");
            display.println("Reiniciando...");
            display.display();
        }
        delay(1000);
        ESP.restart();  // Reiniciar si no hay respuesta
    }

    LoRa.setSpreadingFactor(LORA_SF);
    LoRa.setSignalBandwidth(LORA_BW);
    LoRa.setCodingRate4(LORA_CR);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.setTxPower(LORA_TX_POWER);
    LoRa.enableCrc();

    // Reportar configuración
    Serial.printf("[LoRa] OK — %.3f MHz SF%d BW%.0fkHz CR4/%d\n",
        LORA_FREQ / 1000000.0, LORA_SF, LORA_BW / 1000.0, LORA_CR);
}

//  RUTINA 6: ESPERAR GPS FIX
//  Espera hasta obtener un fix GPS válido o alcanzar timeout.
//    - Lee datos del GPS continuamente
//    - Si hay fix válido y reciente (age < 2000ms), retorna
//    - Si alcanza el timeout (GPS_TIMEOUT_MS), entra en deep sleep
//    - Actualiza la pantalla durante la espera
void esperarGPSFix() {
    unsigned long inicio    = millis();
    unsigned long ultimoLog = 0;

    estadoSistema = "Buscando fix...";
    Serial.println("[GPS] Esperando fix...");

    while (millis() - inicio < GPS_TIMEOUT_MS) {
        // Alimentar el parser de TinyGPS++ con los datos entrantes
        while (SerialGPS.available()) {
            gps.encode(SerialGPS.read());
        }

        // Verificar si hay fix válido y reciente (menos de 2 segundos)
        if (gps.location.isValid() && gps.location.age() < 2000) {
            estadoSistema = "Fix obtenido";
            Serial.printf("[GPS] Fix obtenido en %.1f s — Sat: %d\n",
                (millis() - inicio) / 1000.0,
                gps.satellites.value());
            return;
        }

        // Actualizar pantalla para mostrar satélites visibles
        actualizarPantalla();

        // Log cada 10 segundos mientras espera
        if (millis() - ultimoLog >= 10000) {
            Serial.printf("[GPS] Sin fix... %lu s — Satelites: %d\n",
                (millis() - inicio) / 1000,
                gps.satellites.value());
            ultimoLog = millis();
        }
    }

    // Timeout alcanzado sin fix
    Serial.println("[GPS] Timeout 60s — sin fix, entrando en sleep");
    estadoSistema = "Timeout GPS";
    entrarDeepSleep(false);
}

//  RUTINA 7: LEER DATOS GPS
// Extrae todos los datos del GPS desde la estructura TinyGPS++.
DatosGPS leerDatosGPS() {
    DatosGPS datos;

    // Datos de posición
    datos.latitud         = gps.location.lat();
    datos.longitud        = gps.location.lng();

    // Determinar hemisferios a partir del signo
    datos.hemisferio_lat  = datos.latitud  >= 0 ? 'N' : 'S';
    datos.hemisferio_lon  = datos.longitud >= 0 ? 'E' : 'W';

    // Datos de movimiento
    datos.velocidad_knots = gps.speed.knots();
    datos.rumbo_grados    = gps.course.deg();

    // Datos de altitud
    datos.altitud_metros  = gps.altitude.meters();
    datos.valido          = true;

    // Mostrar en monitor serial para depuración
    Serial.printf("[GPS] Lat: %.6f%c  Lon: %.6f%c\n",
        abs(datos.latitud),  datos.hemisferio_lat,
        abs(datos.longitud), datos.hemisferio_lon);
    Serial.printf("[GPS] Vel: %.1f kt  Rumbo: %.1f  Alt: %.1f m  Sat: %d\n",
        datos.velocidad_knots,
        datos.rumbo_grados,
        datos.altitud_metros,
        gps.satellites.value());

    return datos;
}

//  RUTINA 8: CONVERTIR COORDENADA A FORMATO APRS
// Convierte coordenadas decimales al formato APRS
//  Ejemplo:
//    Latitud 9.853502° -> "0951.21" (9° 51.21 minutos)
//    Longitud -83.920807° -> "08355.25" (83° 55.25 minutos)
String coordToAPRS(float grados, bool esLatitud) {
    float absGrados     = abs(grados); 
    int   gradosEnteros = (int)absGrados;
    float minutos       = (absGrados - gradosEnteros) * 60.0;

    char buffer[12];
    if (esLatitud) {
        sprintf(buffer, "%02d%05.2f", gradosEnteros, minutos);
    } else {
        sprintf(buffer, "%03d%05.2f", gradosEnteros, minutos);
    }

    return String(buffer);
}

//  RUTINA 9: CONSTRUIR TRAMA APRS
// Construye la trama APRS completa según el estándar.
//  Formato APRS para posición:
//    ORIGEN>DESTINO,RUTA:!DDMM.hhN/DDDMM.hhW>RUMBO/VELOCIDAD/A=ALTITUD COMENTARIO
//  
//  Ejemplo de salida:
//    TI0TEC7-7>APRS,WIDE1-1:!0951.21N/08355.25W>/000/002/A=000000 Tracker TEC - Grupo 7
String construirTramaAPRS(DatosGPS datos) {
    // Convertir coordenadas a formato APRS
    String latStr = coordToAPRS(datos.latitud,  true);
    String lonStr = coordToAPRS(datos.longitud, false);

     // Velocidad: 3 dígitos (000-999)
    char velStr[4];
    sprintf(velStr, "%03.0f", datos.velocidad_knots);

    // Rumbo: 3 dígitos (000-359)
    char rumboStr[4];
    sprintf(rumboStr, "%03.0f", datos.rumbo_grados);

    // Altitud: convertir metros a pies (1 m = 3.28084 ft) y formatear a 6 dígitos
    float altitudPies = datos.altitud_metros * 3.28084;
    char  altStr[8];
    sprintf(altStr, "%06.0f", altitudPies);

    // Construir trama completa
    String trama = String(CALLSIGN) +
                   ">APRS,WIDE1-1:!" +              // Encabezado y tipo (! = posición)
                   latStr + datos.hemisferio_lat +  // Latitud DDMM.hhN
                   String(APRS_SYMBOL_TAB) +        // Tabla de símbolos
                   lonStr + datos.hemisferio_lon +  // Longitud DDDMM.hhW
                   String(APRS_SYMBOL) +            // Símbolo (> = vehículo)
                   String(rumboStr) + "/" +         // Rumbo
                   String(velStr) +                 // Velocidad
                   "/A=" + String(altStr) +         // Altitud en pies
                   " " + String(APRS_COMMENT);      // Comentario

    Serial.println("[APRS] Trama: " + trama);
    return trama;
}

//  RUTINA 10: TRANSMITIR POR LORA
// Transmite la trama APRS por LoRa con 3 reintentos.
bool transmitirLoRa(String trama) {
    for (int intento = 1; intento <= MAX_TX_RETRIES; intento++) {
        Serial.printf("[LoRa] Transmitiendo (intento %d/%d)...\n",
            intento, MAX_TX_RETRIES);

        estadoSistema = "Transmitiendo...";
        actualizarPantalla();

        if (LoRa.beginPacket()) {
            LoRa.print(trama);
            if (LoRa.endPacket()) {
                txCount++;
                estadoSistema = "TX OK #" + String(txCount);
                Serial.printf("[LoRa] TX OK — transmision #%d\n", txCount);
                actualizarPantalla();
                delay(500);
                return true;
            }
        }

        Serial.printf("[LoRa] TX fallido en intento %d\n", intento);
        estadoSistema = "TX error " + String(intento) + "/" +
                        String(MAX_TX_RETRIES);
        actualizarPantalla();

        if (intento < MAX_TX_RETRIES) {
            delay(2000);
        }
    }

    estadoSistema = "TX fallido";
    Serial.println("[LoRa] ERROR — reintentos agotados");
    return false;
}

//  RUTINA 11: ESCUCHAR IGATES
//  Después de transmitir, escucha tráfico en la frecuencia
//  para detectar iGates activos en rango.
//  Retorna el callsign del primero que escuche, o "NONE".
String escucharIGates() {
    Serial.printf("[LoRa] Escuchando iGates por %d ms...\n",
        ESCUCHA_IGATES_MS);

    estadoSistema = "Escuchando...";
    actualizarPantalla();

    LoRa.receive();  // Poner SX1276 en modo recepción
    unsigned long inicio = millis();

    while (millis() - inicio < ESCUCHA_IGATES_MS) {
        int paqueteSize = LoRa.parsePacket();

        if (paqueteSize) {
            String paquete = "";
            while (LoRa.available()) {
                paquete += (char)LoRa.read();
            }

            Serial.println("[LoRa Rx] Paquete: " + paquete);

            // Extraer callsign del remitente
            // Formato APRS: CALLSIGN>DEST,...
            int separador = paquete.indexOf('>');
            if (separador > 0) {
                String callsignRx = paquete.substring(0, separador);
                Serial.println("[LoRa Rx] iGate escuchado: " + callsignRx);
                LoRa.idle();
                return callsignRx;
            }
        }

        actualizarPantalla();
    }

    LoRa.idle();  // Salir del modo recepción
    Serial.println("[LoRa] Sin iGates escuchados");
    return "NONE";
}

//  RUTINA 12: ENTRAR EN DEEP SLEEP
//  Pone el ESP32 en deep sleep para maximizar duración de batería.
//  
//  Parámetro mostrarDatosFinales:
//    true:  Muestra la pantalla de posición durante DATOS_FINALES_MS antes de dormir
//    false: Duerme inmediatamente
//
//  Fuentes de wakeup configuradas:
//    1. Timer RTC: Cada SLEEP_DURATION segundos
//    2. Botón GPIO38: Nivel bajo (presionado)
void entrarDeepSleep(bool mostrarDatosFinales) {

    // Mostrar datos de posición por DATOS_FINALES_MS antes de dormir
    if (mostrarDatosFinales && oledDisponible && pantallaOn) {
        Serial.println("[SLEEP] Mostrando datos finales...");
        unsigned long inicio = millis();
        while (millis() - inicio < DATOS_FINALES_MS) {
            mostrarDatosPosicion();
            delay(100);
        }
    }

    estadoSistema = "Sleeping 60s...";
    if (oledDisponible && pantallaOn) {
        mostrarEstadoSistema();
        delay(1000);
    }

    Serial.printf("[SLEEP] Entrando en deep sleep por %d segundos...\n",
        SLEEP_DURATION);
    Serial.flush();

    if (oledDisponible) {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
    }

    // Wakeup por timer RTC (60 segundos)
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000ULL);

    // Wakeup por botón IO38 (presión = pin LOW)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_PIN, 0);

    esp_deep_sleep_start();
}

//  SETUP — Punto de entrada principal
// Punto de entrada principal del programa.
//  Se ejecuta en cada arranque: encendido inicial, wakeup por timer, wakeup por botón.
void setup() {

    // Reducir frecuencia CPU a 80 MHz para ahorrar energía
    // El firmware no requiere alta velocidad de procesamiento
    setCpuFrequencyMhz(CPU_FREQ_MHZ);

    Serial.begin(115200);
    delay(500);

    bootCount++;
    Serial.println("\n=============================");
    Serial.println(" Tracker LoRa-APRS TI0TEC7-7");
    Serial.println("   v4.0 — AXP2101 / 80MHz");
    Serial.println("=============================");
    Serial.printf("Boot #%d\n", bootCount);

    // Detectar causa del arranque
    esp_sleep_wakeup_cause_t causa = esp_sleep_get_wakeup_cause();
    if (causa == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Causa: Wakeup por RTC timer");
    } else if (causa == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Causa: Wakeup por boton IO38");
    } else {
        Serial.println("Causa: Encendido inicial");
    }

    // ── 1. Inicializar hardware ───
    initAXP2101();      // Gestor de energía — siempre primero
    delay(500);         // Esperar estabilización de periféricos
    initOLED();         // Pantalla OLED
    initBoton();        // Botón físico GPIO38
    initGPS();          // UART1 para NEO-M8N
    initLoRa();         // SPI para SX1276

    // ── 2. Esperar fix GPS (sin timeout) ───
    esperarGPSFix();

    // ── 3. Leer datos de posición ────
    datosActuales = leerDatosGPS();
    mostrarDatosPosicion();

    // ── 4. Construir trama APRS ────
    tramaActual = construirTramaAPRS(datosActuales);

    // ── 5. Transmitir por LoRa ───
    bool txExitoso = transmitirLoRa(tramaActual);

    // ── 6. Escuchar iGates después de transmitir ───
    if (txExitoso) {
        ultimoIGate = escucharIGates();
        Serial.println("[LoRa] Ultimo iGate: " + ultimoIGate);
    }

    // ── 7. Deep sleep ───
    // Si TX fue exitoso, mostrar datos finales 5s antes de dormir
    entrarDeepSleep(txExitoso);
}

//  LOOP — Vacío intencionalmente
void loop() {
    // Vacío — deep sleep reinicia desde setup() en cada ciclo
}