/***************************************************************************
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA 
* Carrera        : Ingeniería Electrónica
* Materia        : Inteligencia Artificial
* Docente        : Dr. Juan Alberto Ramírez Quintana
* Unidad         : Unidad 2: Aprendizaje Automático
* Proyecto       : Perceptron
* Archivo        : main.ino
* Dependencias   : BSP.h,GPIO.cpp, ADC.cpp, PRINT.cpp, ML.cpp, RTOS.cpp
* Alumnos        : Luis Adrian Anchondo Carreón (22061088)
*                : Luis Adrian Rodriguez Vargas (22061021)
* Última edición : 23/04/26
* Descripción    : Punto de entrada del programa (Software de Aplicación).
*                  Gestiona los estados OFF / RUN mediante pulsación de 
*                  el botón principal y maneja las funciones de tareas.
*                  El proyecto implementa un perceptron simple de activación
*                  binaria para compuertas AND, OR, y personalizada.
****************************************************************************
*  Estructura del proyecto:
*      BSP.h        - Pines, constantes, tipos, prototipos
*      ADC.cpp      - HAL: Conversor analógico-digital
*      GPIO.cpp     - HAL: Entradas/salidas digitales
*      PRINT.cpp    - HAL: Consola Serial
*      ML.cpp       - Capa ML: Perceptrón + LMS
*      HMI.cpp      - Interfaz de usuario por Serial
*      RTOS.cpp     - Tareas FreeRTOS
*      main.ino     - setup() y loop()
****************************************************************************  
*  Flujo de arranque:
*    1. PRINT_Init      - Inicializar UART
*    2. GPIO_Init       - Configurar LEDs y botón
*    3. HMI_Configure   - Menú: N pots, función lógica, entrenamiento
*    4. RTOS_StartTasks - Lanzar las 3 tareas concurrentes
****************************************************************************
*/

#include "BSP.h"

// ================================================================
//  DEFINICIÓN DE VARIABLES GLOBALES
//  (declaradas extern en BSP.h, definidas aquí exactamente una vez)
// ================================================================

// Mapa de pines ADC para los potenciómetros (orden fijo)
const int g_potPins[BSP_MAX_POTS] = {
    BSP_PIN_POT_0,   // Potenciómetro 1 -> GPIO 34
    BSP_PIN_POT_1,   // Potenciómetro 2 -> GPIO 35
    BSP_PIN_POT_2,   // Potenciómetro 3 -> GPIO 32
    BSP_PIN_POT_3,   // Potenciómetro 4 -> GPIO 33
    BSP_PIN_POT_4    // Potenciómetro 5 -> GPIO 36
};

volatile SysMode_t g_sysMode = SYS_MODE_OFF;             // Estado inicial: OFF
int                g_numPots = 2;                        // Potenciómetros activos
bool               g_mlReady = false;                    // Modelo entrenado?
Perceptron_t       g_model   = {nullptr, 0, 0, false};   // Modelo ML

SemaphoreHandle_t  g_xModeMutex = NULL;   // Mutex para proteger g_sysMode

// ================================================================
//  setup()  Inicialización del sistema (corre una sola vez)
// ================================================================
void setup()
{
    // 1.- Inicializar consola Serial (HAL - PRINT)
    PRINT_Init(BSP_UART_BAUD);

    // 2.- Inicializar GPIO: LEDs y botón (HAL - GPIO)
    GPIO_Init();

    // 3.- Crear el mutex antes de arrancar las tareas
    g_xModeMutex = xSemaphoreCreateMutex();
    if (g_xModeMutex == NULL) 
    {
        Serial.println("[MAIN] ERROR CRITICO: No se pudo crear el mutex.");
        while (true) 
        { 
            delay(1000); 
        }
    }

    // 4.- Menú de configuración + entrenamiento del perceptrón (HMI)
    if (!HMI_Configure()) 
    {
        Serial.println("[MAIN] ERROR: Fallo en la configuracion. Reinicia.");
        while (true) 
        {
            delay(1000); 
        }
    }

    // 5.- Lanzar las tareas FreeRTOS (RTOS)
    RTOS_StartTasks();

    Serial.println("[MAIN] Sistema operativo en ejecucion.");
    Serial.println("[MAIN] Presiona el boton para alternar OFF <-> RUN.");
}

// ================================================================
//  loop()  —  No se usa; FreeRTOS gestiona la concurrencia
// ================================================================
void loop()
{
    // El scheduler de FreeRTOS administra todas las tareas.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
