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


