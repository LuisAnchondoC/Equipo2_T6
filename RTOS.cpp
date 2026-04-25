/***************************************************************************
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA 
* Carrera        : Ingeniería Electrónica
* Materia        : Inteligencia Artificial
* Docente        : Dr. Juan Alberto Ramírez Quintana
* Unidad         : Unidad 2: Aprendizaje Automático
* Proyecto       : Perceptron
* Archivo        : RTOS.cpp
* Dependencias   : BSP.h
* Alumnos        : Luis Adrian Anchondo Carreón (22061088)
*                : Luis Adrian Rodriguez Vargas (22061021)
* Última edición : 23/04/26
* Descripción    : Gestión de Tareas FreeRTOS
*                  Este módulo instancia y lanza las tareas del sistema 
*                  operativo en tiempo real. Cada tarea es un hilo 
*                  independiente con su propia prioridad y stack de memoria.
****************************************************************************/
#include "BSP.h"

// ================================================================
//  TAREA 1: Task_Button
//  Prioridad : 3 (la más alta del sistema — respuesta rápida)
//  Stack     : 2048 bytes
//  Periodo   : BSP_TASK_BTN_MS (20 ms)
//
//  Detecta la pulsación del botón con antirebote y alterna el
//  modo del sistema entre MODE_OFF y MODE_RUN.
//  Usa un mutex para proteger la variable compartida g_sysMode.
//  Controla el LED de estado según el modo activo.
// ================================================================
void Task_Button(void *pvParameters)
{
    bool       lastState = false;
    TickType_t lastPress = 0;

    for (;;) 
    {
        bool pressed = GPIO_ReadPin(BSP_PIN_BUTTON);

        // Detectar flanco de bajada (botón recién presionado)
        if (pressed && !lastState) 
        {
            TickType_t now = xTaskGetTickCount();

            // Antirebote: solo actuar si pasaron BSP_DEBOUNCE_MS
            if ((now - lastPress) >= pdMS_TO_TICKS(BSP_DEBOUNCE_MS)) 
            {
                lastPress = now;

                // Tomar el mutex antes de modificar g_sysMode
                if (xSemaphoreTake(g_xModeMutex, pdMS_TO_TICKS(50)) == pdTRUE) 
                {

                    // Alternar modo
                    g_sysMode = (g_sysMode == SYS_MODE_OFF) ? SYS_MODE_RUN : SYS_MODE_OFF;

                    bool isRun = (g_sysMode == SYS_MODE_RUN);
                    GPIO_WritePin(BSP_PIN_LED_STATE, isRun);

                    xSemaphoreGive(g_xModeMutex);

                    Serial.printf("[BTN] Modo -> %s\n", isRun ? "RUN" : "OFF");
                }
            }
        }

        lastState = pressed;
        vTaskDelay(pdMS_TO_TICKS(BSP_TASK_BTN_MS));
    }
}

// ================================================================
//  TAREA 2: Task_MLInference
//  Prioridad : 2 (media)
//  Stack     : 4096 bytes (mayor porque ML_Predict usa arrays)
//  Periodo   : BSP_TASK_ML_MS (100 ms)
//
//  En modo RUN y con el modelo entrenado:
//    1. Lee los N potenciómetros y binariza sus valores.
//    2. Llama al perceptrón para predecir la salida.
//    3. Enciende o apaga el LED de salida según la predicción.
//  En modo OFF: apaga el LED de salida.
// ================================================================
void Task_MLInference(void *pvParameters)
{
    int inputVec[BSP_MAX_POTS];

    for (;;) {
        if (g_sysMode == SYS_MODE_RUN && g_mlReady) 
        {

            // Leer entradas binarizadas de todos los potenciómetros
            for (int i = 0; i < g_numPots; i++) 
            {
                inputVec[i] = ADC_ReadBinary(i);
            }

            // Inferencia: el perceptrón decide 0 o 1
            int prediction = ML_Predict(&g_model, inputVec);

            // Controlar el LED de salida
            GPIO_WritePin(BSP_PIN_LED_OUTPUT, prediction == 1);

        } 
        else 
        {
            // Modo OFF o modelo no listo -> LED apagado
            GPIO_WritePin(BSP_PIN_LED_OUTPUT, false);
        }

        vTaskDelay(pdMS_TO_TICKS(BSP_TASK_ML_MS));
    }
}

// ================================================================
//  TAREA 3: Task_ADCPrint
//  Prioridad : 1 (la más baja — solo informativa)
//  Stack     : 2048 bytes
//  Periodo   : BSP_TASK_PRINT_MS (500 ms)
//
//  En modo RUN y con el modelo listo, imprime periódicamente
//  las lecturas crudas y binarizadas de los potenciómetros.
//  La baja prioridad garantiza que no interfiera con las tareas
//  de control (botón e inferencia).
// ================================================================
void Task_ADCPrint(void *pvParameters)
{
    for (;;) 
    {
        if (g_sysMode == SYS_MODE_RUN && g_mlReady) 
        {
            PRINT_ADCValues(g_numPots);
        }
        vTaskDelay(pdMS_TO_TICKS(BSP_TASK_PRINT_MS));
    }
}

// ================================================================
//  RTOS_StartTasks
//  Descripción : Crea e inicia todas las tareas del sistema.
//                Debe llamarse desde setup() después de que el
//                hardware y el modelo ML estén inicializados.
// ================================================================
void RTOS_StartTasks(void)
{
    BaseType_t result;

    // xTaskCreate(función, nombre, stackWords, params, prioridad, handle)
    // El scheduler de FreeRTOS ya está corriendo en el ESP32 desde
    // el arranque de Arduino; xTaskCreate registra la tarea.

    result = xTaskCreate(Task_Button,
                         "Boton",       // Nombre (máx 16 chars)
                         2048,          // Stack en bytes
                         NULL,          // Parámetros (no usados)
                         3,             // Prioridad — más alta
                         NULL);         // Handle (no necesario aquí)
    if (result != pdPASS) 
    {
        Serial.println("[RTOS] ERROR: No se pudo crear Task_Button");
    }

    result = xTaskCreate(Task_MLInference,
                         "ML_Infer",
                         4096,          // Stack mayor: arrays locales
                         NULL,
                         2,
                         NULL);
    if (result != pdPASS) 
    {
        Serial.println("[RTOS] ERROR: No se pudo crear Task_MLInference");
    }

    result = xTaskCreate(Task_ADCPrint,
                         "ADC_Print",
                         2048,
                         NULL,
                         1,             // Prioridad — más baja
                         NULL);
    if (result != pdPASS) 
    {
        Serial.println("[RTOS] ERROR: No se pudo crear Task_ADCPrint");
    }

    Serial.println("[RTOS] Tareas instanciadas:");
    Serial.println("[RTOS]   Task_Button      prioridad=3  stack=2048");
    Serial.println("[RTOS]   Task_MLInference prioridad=2  stack=4096");
    Serial.println("[RTOS]   Task_ADCPrint    prioridad=1  stack=2048");
}
