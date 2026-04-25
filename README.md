# Perceptrón LMS embebido en ESP32 + FreeRTOS

> Un perceptrón simple entrenado con el algoritmo LMS (Least Mean Squares) corre sobre un ESP32 con FreeRTOS, clasifica señales analógicas de potenciómetros y refleja su predicción en un LED.

<br>

---

## Tabla de contenidos

- [Descripción](#descripción)
- [Características](#características)
- [Hardware requerido](#hardware-requerido)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Uso por el Monitor Serial](#uso-por-el-monitor-serial)
- [Parámetros configurables](#parámetros-configurables)
- [Cómo funciona el perceptrón](#cómo-funciona-el-perceptrón)
- [Tareas FreeRTOS](#tareas-freertos)
- [Créditos](#créditos)

---

## Descripción

Este proyecto implementa un **perceptrón simple de N entradas** entrenado con el algoritmo **LMS** directamente en un microcontrolador ESP32, sin PC ni librerías de ML externas.

El usuario configura el sistema por puerto serial al arranque:

1. Elige cuántos potenciómetros usar (2 a 5).
2. Elige la función a aprender: `AND`, `OR`, o **tabla de verdad personalizada** definida fila por fila.
3. El sistema entrena automáticamente y queda listo para inferencia en tiempo real.

Al presionar el botón, el sistema entra en modo **RUN**: lee los potenciómetros cada 100 ms, clasifica la entrada con el perceptrón entrenado, y enciende o apaga el LED de salida con la predicción.

---

## Características

- **Entrenamiento en dispositivo:**  el algoritmo LMS corre completamente en el ESP32, sin conexión a un servidor externo.
- **N entradas configurable:** soporta de 2 a 5 potenciómetros como entradas analógicas binarizadas.
- **Tabla personalizada:**  además de AND y OR, el usuario puede definir cualquier función booleana de N variables ingresando las salidas por Serial.
- **FreeRTOS:** tres tareas concurrentes (botón, inferencia, monitor) con prioridades diferenciadas y mutex para variables compartidas.
- **Arquitectura por capas:** HAL / ML / HMI / RTOS claramente separados; un único `BSP.h` centraliza pines, constantes y prototipos.
- **Anti-rebote:** detección de flancos con ventana de 200 ms para el botón.

---

## Hardware requerido

| Componente | Cantidad | Notas |
|---|---|---|
| ESP32 DevKit v1 (o compatible) | 1 | Dual-core Xtensa LX6, 240 MHz |
| Potenciómetro 10 kΩ | 2 – 5 | Uno por entrada del perceptrón |
| LED + resistencia 220 Ω | 2 | Estado del sistema y salida ML |
| Botón pulsador | 1 | Con resistencia pull-up interna del ESP32 |
| Protoboard + cables Dupont | — | Para el montaje |

---


## Estructura del proyecto

```
perceptron-esp32/
│
├── Equipo2_T6/
│   ├── main.ino        # Punto de entrada: setup() y loop()
│   ├── BSP.h           # Board Support Package: pines, constantes, tipos, prototipos
│   ├── ADC.cpp         # HAL: lectura y binarización de potenciómetros
│   ├── GPIO.cpp        # HAL: control de LEDs y lectura del botón
│   ├── PRINT.cpp       # HAL: consola serial (banner, valores ADC, separadores)
│   ├── HMI.cpp         # Interfaz hombre-máquina: configuración interactiva por Serial
│   ├── ML.cpp          # Perceptrón LMS: init, generación de tabla, train, predict, verify
│   └── RTOS.cpp        # Tareas FreeRTOS: botón, inferencia ML, impresión ADC
│
└── README.md
```


## Uso por el Monitor Serial

Al arrancar, el sistema entra en modo de configuración interactiva:

```
================================================
  Perceptron Simple con LMS — ESP32 + FreeRTOS
  Laboratorio de Machine Learning  |  ITCH
================================================

[HMI] ============ CONFIGURACION DEL SISTEMA ============
[HMI] Cuantos potenciometros vas a usar?
      Ingresa un numero entre 2 y 5, luego ENTER:
> 2

[HMI] Que funcion logica deseas entrenar?
      1 = AND
      2 = OR
      3 = PERSONALIZADA  (defines tu propia tabla de verdad)
> 1

[ML]  Tabla de verdad  |  X1  X2  |  Y
------------------------------------------------
                        0   0   |  0
                        0   1   |  0
                        1   0   |  0
                        1   1   |  1
------------------------------------------------
[ML]  Iniciando entrenamiento LMS...
[ML]  Convergencia en 12 epocas (3 ms)
[ML]  Pesos finales: w1=0.5012  w2=0.4998  bias=-0.7501
[ML]  Precision: 4 / 4  (100%)

[HMI] Sistema entrenado y listo.
[HMI] Presiona el boton (GPIO 18) para activar modo RUN.
```

### Modo tabla personalizada

Al elegir la opción `3`, el sistema solicita la salida para cada combinación:

```
[HMI] Ingresa la salida deseada (0 o 1) para cada combinacion de entradas.
------------------------------------------------
  X1  X2  →  Y (escribe 0 o 1)
------------------------------------------------
  [0,0]  →  0    (guardado: 0)
  [0,1]  →  1    (guardado: 1)
  [1,0]  →  1    (guardado: 1)
  [1,1]  →  0    (guardado: 0)
```

> Si se define una función **no linealmente separable** (como XOR), el perceptrón simple no podrá aprenderla. El sistema lo notificará al alcanzar el límite de épocas y continuará con la mejor aproximación encontrada.

### En modo RUN

```
[BTN] Modo → RUN
[ADC] Raw:  P1=3892  P2= 312  | Binario: [1, 0]
[ADC] Raw:  P1=3901  P2= 287  | Binario: [1, 0]
...
```

El LED de salida (GPIO 2) se enciende o apaga según la predicción del perceptrón. Presionar el botón alterna entre OFF y RUN.

---

## Parámetros configurables

Todos los parámetros del sistema se encuentran en `BSP.h`:

| Constante | Valor | Descripción |
|---|---|---|
| `BSP_LMS_ETA` | `0.5` | Tasa de aprendizaje η del algoritmo LMS |
| `BSP_LMS_MIN_ERROR` | `0.01` | Umbral de convergencia: \|e\| < 0.01 para todos los ejemplos |
| `BSP_LMS_MAX_EPOCHS` | `500` | Límite de épocas antes de detener el entrenamiento |
| `BSP_ADC_THRESHOLD` | `2047` | Umbral de binarización ADC (mitad del rango de 12 bits) |
| `BSP_DEBOUNCE_MS` | `200` | Tiempo mínimo entre pulsaciones válidas del botón |
| `BSP_TASK_BTN_MS` | `20` | Período de muestreo del botón (50 Hz) |
| `BSP_TASK_ML_MS` | `100` | Período de inferencia (10 Hz) |
| `BSP_TASK_PRINT_MS` | `500` | Período de impresión serial (2 Hz) |
| `BSP_UART_BAUD` | `115200` | Velocidad del puerto serial |

---

## Cómo funciona el perceptrón

El perceptrón calcula la **suma ponderada** de sus entradas y aplica una función escalón:

```
g(x) = w₁·x₁ + w₂·x₂ + … + wₙ·xₙ + w₀

ŷ = hardlim(g) = { 1  si g ≥ 0.5
                 { 0  si g <  0.5
```

El **algoritmo LMS** ajusta los pesos en cada ejemplo de entrenamiento:

```
e     = y_real − ŷ
wₖ   += η · e · xₖ    (para cada entrada k)
w₀   += η · e          (bias, sin multiplicar por entrada)
```

El entrenamiento converge cuando `|e| < 0.01` para **todos** los ejemplos, o se detiene al alcanzar 500 épocas.

### Limitación fundamental

Un perceptrón simple solo puede aprender funciones **linealmente separables** — aquellas donde un hiperplano puede dividir las clases. AND y OR lo son; XOR no. Para funciones no separables se requiere una red multicapa (fuera del alcance de este proyecto).

---

## Tareas FreeRTOS

| Tarea | Prioridad | Período | Stack | Responsabilidad |
|---|---|---|---|---|
| `Task_Button` | 3 (alta) | 20 ms | 2 048 B | Detecta pulsaciones con anti-rebote, alterna modo OFF ↔ RUN, controla LED de estado |
| `Task_MLInference` | 2 (media) | 100 ms | 4 096 B | Lee pots, binariza, llama `ML_Predict()`, controla LED de salida |
| `Task_ADCPrint` | 1 (baja) | 500 ms | 2 048 B | Imprime lecturas raw y binarias de los potenciómetros por Serial |

La variable `g_sysMode` es protegida por el mutex `g_xModeMutex` cuando `Task_Button` la modifica.

---

## Créditos

- Algoritmo LMS basado en `lmsNdim.m` del **Dr. Juan Ramírez** (28 dic 2016).
- Desarrollado para el **Laboratorio de Machine Learning — Tarea 6**, Instituto Tecnológico de Chihuahua.
- Framework: [Arduino-ESP32](https://github.com/espressif/arduino-esp32) por Espressif Systems.
- RTOS: [FreeRTOS](https://www.freertos.org/) — integrado en el SDK del ESP32.

---


