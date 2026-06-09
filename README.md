# Grupo7_Tracker5
# GPS LoRa APRS Tracker para Monitoreo de Bicicletas

## Descripción del proyecto

Este proyecto consiste en el desarrollo de un sistema de rastreo de posición para bicicletas basado en tecnologías de comunicación de largo alcance y bajo consumo energético. El objetivo principal es implementar el firmware de un módulo *tracker* capaz de adquirir información de posicionamiento mediante un módulo GPS y transmitirla periódicamente utilizando comunicación LoRa en formato compatible con APRS.

La solución está orientada a escenarios donde la cobertura de redes celulares puede ser limitada o inexistente, como rutas rurales o zonas montañosas. Mediante el uso de tecnologías de radiofrecuencia en bandas libres, el sistema permite enviar información básica de ubicación sin depender de infraestructura de operadores móviles.

---

## Objetivo general

Desarrollar el firmware de un dispositivo *tracker* que permita adquirir datos de posicionamiento GPS y transmitirlos mediante LoRa utilizando el protocolo APRS.

---

## Objetivos específicos

- Procesar datos de posicionamiento provenientes de un módulo GPS.
- Implementar un parser para interpretar tramas NMEA.
- Generar tramas de datos compatibles con el formato APRS.
- Configurar y controlar la transmisión mediante un módulo LoRa.

---

## Tecnologías utilizadas

Este proyecto integra diferentes tecnologías de hardware y comunicación:

- **GPS**  
  Proporciona información de posicionamiento geográfico mediante tramas NMEA.

- **LoRa (Long Range)**  
  Tecnología de comunicación inalámbrica de largo alcance y bajo consumo energético.

- **APRS (Automatic Packet Reporting System)**  
  Protocolo utilizado para transmitir información de posición y telemetría en redes de radio.

- **T-Beam / Meshtastic**  
  Plataforma de hardware que integra microcontrolador, módulo LoRa y GPS en un mismo dispositivo.

---

## Funcionamiento general del sistema

El sistema opera de la siguiente manera:

1. El módulo GPS obtiene la posición geográfica del dispositivo.
2. El firmware recibe las tramas NMEA generadas por el GPS.
3. Se procesa la información para verificar la validez del posicionamiento.
4. Se extraen las coordenadas de latitud y longitud.
5. El sistema genera una trama en formato APRS.
6. Finalmente, la información es transmitida mediante el módulo LoRa a intervalos periódicos.

Este proceso permite enviar la ubicación del dispositivo de forma continua utilizando enlaces de radio de largo alcance.

---

## Estructura del repositorio

```
/firmware
    Código fuente del firmware del tracker

/docs
    Documentación técnica del proyecto y diagramas

/test
    Scripts o pruebas de funcionamiento
```

---

## Estado del proyecto

Actualmente el proyecto se encuentra en fase de desarrollo académico, donde se están realizando las siguientes actividades:

- Investigación de las tecnologías involucradas
- Diseño de la arquitectura del sistema
- Planificación del desarrollo del firmware
- Implementación progresiva de los módulos del sistema

---

## Aplicaciones potenciales

- Monitoreo de ciclistas en rutas rurales o de montaña
- Sistemas de rastreo en zonas con baja cobertura celular
- Proyectos educativos de comunicaciones inalámbricas
- Sistemas de telemetría de bajo consumo energético

## 📥 Instalación y compilación del firmware

### Requisitos previos

1. **PlatformIO** (no Arduino IDE)
   - Descargar desde [platformio.org](https://platformio.org/)
   - Opciones recomendadas:
     - **VS Code**: Instalar la extensión "PlatformIO IDE"
     - **CLI**: Usar `pip install platformio` en terminal

2. **Hardware necesario**
   - Placa T-Beam V1.2 (ESP32 + SX1276 + NEO-M8N + AXP2101)
   - Cable USB-C (o Micro-USB según la versión de tu placa)
   - Batería LiPo (opcional, para pruebas en campo)

3. **Librerías** (se instalan automáticamente con PlatformIO)
   - `TinyGPSPlus` - Para parsear las tramas NMEA del GPS
   - `LoRa` - Para controlar el módulo SX1276
   - `XPowersLib` - Para gestionar el chip de energía AXP2101
   - `Adafruit GFX` y `SSD1306` - Para controlar la pantalla OLED

### Pasos de instalación

#### 1. Clonar el repositorio
```bash
git clone https://github.com/shuz1222/Grupo7_Tracker5.git
cd Grupo7_Tracker5/firmware
```
#### 2. Abrir el proyecto en PlatformIO
- En VS Code: Archivo → Abrir carpeta y seleccionar la carpeta firmware/

- La extensión PlatformIO detectará automáticamente el archivo platformio.ini

#### 3. Configurar el callsign (opcional)
Editar el archivo src/main.cpp y modificar esta línea:
```cpp
#define CALLSIGN "TI0TEC7-7"   // Cambia por tu indicativo
```
#### 4. Compilar el firmware
```bash
pio run
```
O usar el botón ✓ (Compilar) en la barra inferior de PlatformIO.

#### 5. Subir (flashear) el firmware al T-Beam
```bash
pio run --target upload
```
O usar el botón → (Subir) en la barra inferior de PlatformIO.

#### 6. Monitorear la salida serie
```bash
pio device monitor --baud 115200
```
O usar el botón 🔌 (Serial Monitor) en PlatformIO.

### Verificación de funcionamiento
Al encender el T-Beam, deberías ver en el monitor serie algo similar a:
```text
=============================
 Tracker LoRa-APRS TI0TEC7-7
      AXP2101 / 80MHz
=============================
[AXP2101] OK — Bat: 4.13V  100%
[OLED] OK — pantalla inicializada
[GPS] Esperando fix...
[LoRa] OK — 433.775 MHz SF12 BW125kHz CR4/5
[GPS] Fix obtenido en 2.3 s — Sat: 8
[LoRa] TX OK — transmision #1
```

### Estructura del firmware
```text
firmware/
├── platformio.ini              # Configuración del proyecto
├── src/
│   └── main.cpp                # Código fuente completo
└── Firmware.code-workspace     # Configuración de VS Code (opcional)
```

### Solución de problemas comunes
| Problema |	Posible solución |
|-----------|-----------|
| LoRa ERROR — SX1276 no responde |	Verificar conexiones SPI o reiniciar la placa |
| GPS no obtiene fix |	Llevar el tracker a exteriores con cielo despejado |
| Error de compilación con XPowersLib |	Actualizar la librería o verificar la versión de PlatformIO |
| El tracker no transmite |	Verificar la frecuencia y Sync Word en platformio.ini |


