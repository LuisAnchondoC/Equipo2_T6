/***************************************************************************
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA 
* Carrera        : Ingeniería Electrónica
* Materia        : Inteligencia Artificial
* Docente        : Dr. Juan Alberto Ramírez Quintana
* Unidad         : Unidad 2: Aprendizaje Automático
* Proyecto       : Perceptron
* Archivo        : HMI.h
* Dependencias   : BSP.h
* Alumnos        : Luis Adrian Anchondo Carreón (22061088)
*                : Luis Adrian Rodriguez Vargas (22061021)
* Última edición : 23/04/26
* Descripción    : Interfaz Hombre-Máquina por Puerto Serial
*                  - Guia al usuario para configurar el número de potenciómetros
*                  - Selecciona la función lógica (AND / OR / PERSONALIZADA)
*                  - Captura tabla de verdad personalizada via Serial (opción 3)
*                  - Lanza el entrenamiento del perceptrón
*                  - Muestra resultados de entrenamiento y verificación
****************************************************************************/
#include "BSP.h"

// ----------------------------------------------------------------
//  Función auxiliar privada: leer un entero desde Serial
//  Bloquea hasta que el usuario envía datos.
// ----------------------------------------------------------------
static int readIntFromSerial(void)
{
    // Esperar a que lleguen datos al buffer
    while (!Serial.available()) 
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    int val = Serial.parseInt();
    // Vaciar el resto del buffer (retorno de carro, etc.)
    while (Serial.available()) Serial.read();
    return val;
}

// ----------------------------------------------------------------
//  HMI_Configure
//  Descripción : Flujo completo de configuración interactiva.
//                1. Solicita número de potenciómetros (2–5)
//                2. Solicita función lógica (AND / OR / PERSONALIZADA)
//                3. Para opción PERSONALIZADA: solicita cada salida
//                   de la tabla de verdad por Serial
//                4. Genera / captura la tabla de verdad
//                5. Inicializa y entrena el perceptrón
//                6. Verifica el modelo
//  Retorna     : true si el proceso fue exitoso, false si hubo error
// ----------------------------------------------------------------
bool HMI_Configure(void)
{
    int nPots = 0;
    int sel   = 0;

    // ── PASO 1: Número de potenciómetros ─────────────────────────
    Serial.println("[HMI] ============ CONFIGURACION DEL SISTEMA ============");
    Serial.println("[HMI] Cuantos potenciometros vas a usar?");
    Serial.println("      Ingresa un numero entre 2 y 5, luego ENTER:");

    while (nPots < 2 || nPots > BSP_MAX_POTS) 
    {
        nPots = readIntFromSerial();
        if (nPots < 2 || nPots > BSP_MAX_POTS) 
        {
            Serial.printf("[HMI] Valor invalido (%d). Ingresa entre 2 y %d:\n", nPots, BSP_MAX_POTS);
        }
    }

    g_numPots = nPots;
    Serial.printf("[HMI] OK — Potenciometros seleccionados: %d\n", g_numPots);

    // Imprimir qué pines se van a usar
    Serial.print("[HMI] Pines ADC activos: ");
    for (int i = 0; i < g_numPots; i++) {
        Serial.printf("GPIO%d ", g_potPins[i]);
    }
    Serial.println();

    // PASO 2: Función lógica
    Serial.println("[HMI] Que funcion logica deseas entrenar?");
    Serial.println("      1 = AND");
    Serial.println("      2 = OR");
    Serial.println("      3 = PERSONALIZADA  (defines tu propia tabla de verdad)");
    Serial.println("      Escribe 1, 2 o 3, luego ENTER:");

    while (sel != 1 && sel != 2 && sel != 3) 
    {
        sel = readIntFromSerial();
        if (sel != 1 && sel != 2 && sel != 3) 
        {
            Serial.println("[HMI] Valor invalido. Escribe 1 (AND), 2 (OR) o 3 (PERSONALIZADA):");
        }
    }

    LogicFunc_t func;
    switch (sel) 
    {
        case 1:  func = LOGIC_AND;    Serial.println("[HMI] OK - Funcion: AND");           break;
        case 2:  func = LOGIC_OR;     Serial.println("[HMI] OK - Funcion: OR");            break;
        default: func = LOGIC_CUSTOM; Serial.println("[HMI] OK - Funcion: PERSONALIZADA"); break;
    }

    // PASO 3: Inicializar ADC con los canales elegidos
    ADC_Init(g_numPots);

    // PASO 4: Generar / capturar tabla de verdad
    int  numRows;
    int *table = nullptr;

    if (func == LOGIC_CUSTOM) 
    {
        // Opción personalizada
        int totalRows = 1 << g_numPots;     // 2^N combinaciones
        Serial.println("[HMI] -----------------------------------------------");
        Serial.printf("[HMI] Tu tabla tendra %d filas (%d entradas -> 2^%d combinaciones).\n",
                      totalRows, g_numPots, g_numPots);
        Serial.println("[HMI] Se mostrara cada combinacion de entradas y");
        Serial.println("[HMI] debes escribir la salida deseada (0 o 1) + ENTER.");
        Serial.println("[HMI] -----------------------------------------------");

        table = ML_GenerateCustomTable(g_numPots, &numRows);

    } 
    else 
    {
        // AND / OR generados automáticamente
        Serial.println("[HMI] Generando tabla de verdad...");
        table = ML_GenerateTruthTable(g_numPots, func, &numRows);
    }

    if (table == nullptr) 
    {
        Serial.println("[HMI] ERROR: No se pudo generar la tabla de verdad.");
        return false;
    }

    // Mostrar la tabla final (sea cual sea el modo)
    ML_PrintTruthTable(table, numRows, g_numPots);

    // PASO 5: Inicializar y entrenar el perceptrón
    ML_Init(&g_model, g_numPots);
    ML_Train(&g_model, table, numRows);

    // PASO 6: Verificar el modelo con la tabla completa
    ML_Verify(&g_model, table, numRows);

    free(table);    // Liberar tabla de verdad (ya no se necesita)

    // Listo
    g_mlReady = true;
    Serial.println("[HMI] Sistema entrenado y listo.");
    Serial.println("[HMI] Presiona el boton (GPIO 0) para activar modo RUN.");
    PRINT_Separator();

    return true;
}
