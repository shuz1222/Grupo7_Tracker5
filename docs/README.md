# 📁 Documentación del Proyecto

Esta carpeta contiene toda la documentación técnica, diagramas, informes y recursos complementarios generados durante el desarrollo del **Tracker LoRa-APRS para ciclismo de montaña**.

---
## 📂 Estructura de la carpeta docs/

- **Agro_LoRa/** - Trabajo extra: Monitoreo agrícola de bajo consumo
- **LoRa_APRS_Tracker-main/** - Firmware de referencia (Richonguzman/CA2RXU)
- **avance/** - Primer avance del proyecto
- **diagramas/** - Diagramas eléctricos y máquina de estados
- **fig/** - Recursos gráficos (Diagrama de Gantt)
- **Presentacion_informe_1.pdf** - Presentación del primer informe
- **informe_1.pdf** - Informe técnico completo (primer avance)


---

## 📂 Descripción detallada

### 🌱 Agro_LoRa
Trabajo complementario que explora el **diseño de un sistema de monitoreo agrícola de bajo consumo** utilizando tecnología LoRa. Incluye:
- Justificación del uso de LoRa en entornos rurales
- Diagrama de bloques del sistema
- Estimación de consumo energético
- Consideraciones para despliegue en campo

> Este anexo demuestra la versatilidad de la tecnología LoRa más allá del tracking vehicular.

---

### 📡 LoRa_APRS_Tracker-main
Firmware de referencia desarrollado por **Ricardo Guzmán (CA2RXU)** , utilizado como punto de partida y comparación para nuestro desarrollo. Contiene:
- Código fuente completo en C++ para ESP32
- Configuración para múltiples regiones (EU, US, UK, POLAND)
- Implementación de iGate y digipeater
- Sistema de logging y gestión de energía

> Este firmware fue probado inicialmente para validar conceptos y comparar rendimiento con nuestra implementación propia.

---

### 📝 avance/
Primer entregable del proyecto, donde se establecen las bases conceptuales y técnicas:

| Contenido |
|-----------|
| Definición del problema: monitoreo de ciclistas en zonas de montaña sin cobertura celular |
| Diagrama esquemático del T-Beam con conexiones de periféricos |
| Desglose de costos de componentes y materiales |
| Algoritmo principal en pseudocódigo antes de implementación |

---

### 📐 diagramas/
Recursos visuales que documentan el diseño y flujo del sistema:

| Archivo | Descripción |
|---------|-------------|
| `diagrama_bloques.*` | Diagrama de bloques funcional del tracker |
| `diagrama_electrico.*` | Conexiones eléctricas detalladas (pines, buses I²C/SPI, alimentación) |
| `maquina_estados.*` | Máquina de estados del firmware |

---

### 📊 fig/
Recursos gráficos utilizados en la documentación:
- `diagrama_gantt.*`: Cronograma de actividades de la investigación inicial (elaborado en LaTeX)

---

### 📄 Informes principales

| Archivo | Descripción |
|---------|-------------|
| `Presentacion_informe_1.pdf` | Presentación en diapositivas de investigación preliminar (definición del problema, arquitectura, plan de trabajo) |
| `informe_1.pdf` | Informe técnico completo de la investigación preliminar, incluyendo justificación, marco teórico, diseño preliminar y cronograma |

---


