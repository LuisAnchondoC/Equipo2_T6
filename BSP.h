/***************************************************************************
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA 
* Carrera        : Ingeniería Electrónica
* Materia        : Inteligencia Artificial
* Docente        : Dr. Juan Alberto Ramírez Quintana
* Unidad         : Unidad 2: Aprendizaje Automático
* Proyecto       : Perceptron
* Archivo        : BSP.h
* Dependencias   : Arduino.h, FreeRTOS.h, task.h, semphr.h
* Alumnos        : Luis Adrian Anchondo Carreón (22061088)
*                : Luis Adrian Rodriguez Vargas (22061021)
* Última edición : 23/04/26
* Descripción    : Board Support Package,Centraliza todos los pines, 
*                  constantes y prototipos del proyecto.
*                  Cualquier archivo .cpp debe incluir solo este header.
****************************************************************************/
#ifndef BSP_H
#define BSP_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// ----------------------------------------------------------------
//  SECCIÓN 1: PINES DE HARDWARE
// ----------------------------------------------------------------

// LEDs
#define BSP_PIN_LED_STATE   4   // LED indicador de modo OFF / RUN
#define BSP_PIN_LED_OUTPUT  2   // LED salida del perceptrón

// Botón
#define BSP_PIN_BUTTON     18   // GPIO 0 (BOOT) — pull-up interno

// Potenciómetros — pines ADC solo-entrada del ESP32
#define BSP_PIN_POT_0      13
#define BSP_PIN_POT_1      32
#define BSP_PIN_POT_2      33
#define BSP_PIN_POT_3      35
#define BSP_PIN_POT_4      34

// ----------------------------------------------------------------
//  SECCIÓN 2: CONSTANTES DEL SISTEMA
// ----------------------------------------------------------------

// ADC
#define BSP_ADC_RESOLUTION  12      // Bits de resolución (0–4095)
#define BSP_ADC_MAX         4095    // Valor máximo del ADC
#define BSP_ADC_THRESHOLD   2047    // Umbral para binarizar (≥ → 1)

// Potenciómetros
#define BSP_MAX_POTS        5       // Número máximo soportado

// Debounce del botón
#define BSP_DEBOUNCE_MS     200     // ms mínimos entre pulsaciones

// Retardos de tareas RTOS
#define BSP_TASK_BTN_MS     20      // Polling del botón
#define BSP_TASK_ML_MS      100     // Ciclo de inferencia
#define BSP_TASK_PRINT_MS   500     // Impresión de lecturas ADC

// Puerto serie
#define BSP_UART_BAUD       115200  //Velocidad de comunicación

// ----------------------------------------------------------------
//  SECCIÓN 3: CONSTANTES DE LA CAPA ML
// ----------------------------------------------------------------

#define BSP_LMS_ETA         0.5f    // Tasa de aprendizaje
#define BSP_LMS_MIN_ERROR   0.01f   // Umbral de convergencia
#define BSP_LMS_MAX_EPOCHS  500     // Máximo de épocas

// ----------------------------------------------------------------
//  SECCIÓN 4: TIPOS Y ENUMERACIONES GLOBALES
// ----------------------------------------------------------------

// Modo operativo del sistema
typedef enum 
{
    SYS_MODE_OFF = 0,
    SYS_MODE_RUN = 1
} SysMode_t;

// Función lógica que entrena el perceptrón
typedef enum 
{
    LOGIC_AND    = 1,
    LOGIC_OR     = 2,
    LOGIC_CUSTOM = 3    // Salidas definidas por el usuario via Serial
} LogicFunc_t;

// Modelo del perceptrón (memoria dinámica)
typedef struct 
{
    float  *weights;    // Arreglo [w1 … wN | w0(bias)]
    int     dim;        // Número de entradas = número de pots
    int     epochs;     // Épocas de entrenamiento realizadas
    bool    trained;    // ¿Modelo entrenado?
} Perceptron_t;

// ----------------------------------------------------------------
//  SECCIÓN 5: VARIABLES GLOBALES COMPARTIDAS
// ----------------------------------------------------------------
//  Declaradas como extern aquí; definidas en main.ino
// ----------------------------------------------------------------

extern volatile SysMode_t g_sysMode;    // Modo actual del sistema
extern int                g_numPots;    // Potenciómetros activos (2–5)
extern bool               g_mlReady;    // Perceptrón entrenado y listo
extern Perceptron_t       g_model;      // Instancia del modelo ML
extern const int          g_potPins[BSP_MAX_POTS]; // Pines ADC activos

// Objeto de sincronización (definido en main.ino)
extern SemaphoreHandle_t  g_xModeMutex;

// ----------------------------------------------------------------
//  SECCIÓN 6: PROTOTIPOS ADC  (ADC.cpp)
// ----------------------------------------------------------------

void ADC_Init(int numChannels);
int  ADC_ReadRaw(int channel);
int  ADC_ReadBinary(int channel);

// ----------------------------------------------------------------
//  SECCIÓN 7: PROTOTIPOS GPIO  (GPIO.cpp)
// ----------------------------------------------------------------

void GPIO_Init(void);
void GPIO_WritePin(int pin, bool value);
bool GPIO_ReadPin(int pin);

// ----------------------------------------------------------------
//  SECCIÓN 8: PROTOTIPOS PRINT  (PRINT.cpp)
// ----------------------------------------------------------------

void PRINT_Init(int baudRate);
void PRINT_Separator(void);
void PRINT_ADCValues(int numChannels);
void PRINT_Banner(void);

// ----------------------------------------------------------------
//  SECCIÓN 9: PROTOTIPOS ML Layer  (ML.cpp)
// ----------------------------------------------------------------

void  ML_Init(Perceptron_t *p, int dim);
int*  ML_GenerateTruthTable(int dim, LogicFunc_t func, int *numRows);
int*  ML_GenerateCustomTable(int dim, int *numRows);   // <- NUEVO: tabla con salidas del usuario
void  ML_PrintTruthTable(const int *table, int numRows, int dim);
void  ML_Train(Perceptron_t *p, const int *table, int numRows);
int   ML_Predict(const Perceptron_t *p, const int *x);
void  ML_Verify(const Perceptron_t *p, const int *table, int numRows);
void  ML_Free(Perceptron_t *p);

// ----------------------------------------------------------------
//  SECCIÓN 10: PROTOTIPOS HMI  (HMI.cpp)
// ----------------------------------------------------------------

bool HMI_Configure(void);

// ----------------------------------------------------------------
//  SECCIÓN 11: PROTOTIPOS TAREAS RTOS  (RTOS.cpp)
// ----------------------------------------------------------------

void Task_Button(void *pvParameters);
void Task_MLInference(void *pvParameters);
void Task_ADCPrint(void *pvParameters);
void RTOS_StartTasks(void);

#endif
