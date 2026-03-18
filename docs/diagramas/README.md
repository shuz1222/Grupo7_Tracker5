# Diagramas del sistema — Tracker LoRa-APRS

## Diagrama de bloques — Nivel 1
Vista general del sistema con entradas y salidas principales.

![Nivel 1](Diagrama primer nivel.svg)

### Entradas:
-	Señal GPS: Señal de radiofrecuencia proveniente de los satélites GNSS, recibida por la antena GPS integrada del T-Beam.
-	Energía: Suministro de potencia proporcionado por la batería 18650 a través del gestor de energía AXP192.
-	Firmware/parámetros: Parámetros de configuración (callsign, frecuencia, intervalo TX) ingresados desde una PC vía USB durante la programación. 
### Salidas:
-	Trama LoRa RF: Paquete APRS transmitido en formato LoRa a 433.775 MHz con la posición, velocidad y rumbo del tracker.
-	Debug serial: Información de estado del sistema enviada por UART0/USB para monitoreo durante desarrollo.
-	Indicadores: LED1 indica estado de carga de batería; LED2 indica actividad del GPS.


## Diagrama de bloques — Nivel 2
Módulos internos del sistema y flujo de datos entre ellos.

![Nivel 2](Diagrama segundo nivel.svg)

### Módulo de gestión de energía:
Es un módulo pasivo respecto al flujo lógico del sistema. Su única responsabilidad es garantizar alimentación estable. No toma decisiones ni genera señales de control.
### Módulo de adquisición GPS:
Genera tramas NMEA continuamente a 1 Hz. Cada trama indica si el fix es válido (A) o no (V). El módulo de procesamiento es quien interpreta ese campo, no el GPS.
### Módulo de procesamiento:
Es el núcleo lógico del sistema. Ejecuta un bucle de lectura GPS con timeout de 60 segundos. Si obtiene fix antes del timeout, construye la trama y la envía al módulo LoRa. Al recibir TX done, o al cumplirse el timeout sin fix, ordena al control de ciclo que inicie el deep sleep.
### Módulo de transmisión LoRa:
Recibe la trama APRS, la transmite con los parámetros LoRa configurados y notifica al procesamiento cuando la transmisión concluye.
### Módulo de control de ciclo:
Al recibir la orden de sleep, configura el timer Real Time Clock (RTC) interno del ESP32 con el intervalo definido y ejecuta el deep sleep. El RTC es el único componente que permanece activo durante el reposo. Al cumplirse el tiempo, reinicia el ESP32 y el ciclo comienza nuevamente desde el procesamiento.
