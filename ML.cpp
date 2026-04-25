/***************************************************************************
* INSTITUTO TECNOLÓGICO DE CHIHUAHUA 
* Carrera        : Ingeniería Electrónica
* Materia        : Inteligencia Artificial
* Docente        : Dr. Juan Alberto Ramírez Quintana
* Unidad         : Unidad 2: Aprendizaje Automático
* Proyecto       : Perceptron
* Archivo        : ML.cpp
* Dependencias   : BSP.h
* Alumnos        : Luis Adrian Anchondo Carreón (22061088)
*                : Luis Adrian Rodriguez Vargas (22061021)
* Última edición : 23/04/26
* Descripción    : Capa de Machine Learning, Perceptrón Simple con LMS.
* ==========================================================================
*  Basado en lmsNdim.m del Dr. Juan Ramírez (28 dic 2016)
* ==========================================================================
*  El perceptrón implementa la función discriminante lineal:
*      g(x) = w1·x1 + w2·x2 + ... + wN·xN + w0
*
*  El algoritmo LMS (Least Mean Squares) ajusta los pesos con:
*      w_k(t+1) = w_k(t) + n · e(t) · x_k
*      w_0(t+1) = w_0(t) + n · e(t)          (bias)
*  donde e(t) = y_real - hardlim(g(x))
****************************************************************************/

#include "BSP.h"

// ----------------------------------------------------------------
//  Función de activación escalón unitario (hardlim)
//  Retorna 1 si v >= 0.5, 0 en caso contrario.
//  Se usa un umbral de 0.5 para estabilidad numérica.
// ----------------------------------------------------------------
static int hardlim(float v)
{
    return (v >= 0.5f) ? 1 : 0;
}

// ----------------------------------------------------------------
//  ML_Init
//  Descripción : Inicializa el perceptrón con dimensión `dim`.
//                Reserva memoria dinámica para los N+1 pesos y
//                los inicializa con valores aleatorios en [0, 1).
//  Parámetros  : p   — puntero al modelo
//                dim — número de entradas (= número de pots)
// ----------------------------------------------------------------
void ML_Init(Perceptron_t *p, int dim)
{
    p->dim     = dim;
    p->trained = false;
    p->epochs  = 0;

    // Liberar memoria previa si el modelo fue reinicializado
    if (p->weights != nullptr) 
    {
        free(p->weights);
    }

    // dim pesos + 1 bias → (dim+1) floats
    p->weights = (float*) malloc((dim + 1) * sizeof(float));

    if (p->weights == nullptr) 
    {
        Serial.println("[ML] ERROR: No hay memoria heap para los pesos.");
        return;
    }

    // Inicializar pesos aleatorios entre 0.0 y 1.0
    randomSeed(esp_random());
    for (int i = 0; i <= dim; i++) 
    {
        p->weights[i] = (float)random(0, 1000) / 1000.0f;
    }

    Serial.printf("[ML] Perceptron inicializado: %d entrada(s) + 1 bias\n", dim);
    Serial.print("[ML] Pesos iniciales: ");
    for (int i = 0; i < dim; i++) 
    {
        Serial.printf("w%d=%.3f  ", i + 1, p->weights[i]);
    }
    Serial.printf("bias=%.3f\n", p->weights[dim]);
}

// ----------------------------------------------------------------
//  ML_GenerateTruthTable
//  Descripción : Genera la tabla de verdad para AND u OR con N
//                variables. Usa memoria dinámica.
//                La tabla tiene 2^N filas y (N+1) columnas.
//                La última columna es la salida deseada.
//  Parámetros  : dim     — número de variables (= pots)
//                func    — LOGIC_AND o LOGIC_OR
//                numRows — salida: número de filas generadas
//  Retorna     : Puntero al arreglo aplanado (filas × cols)
//                El llamador debe liberar con free().
// ----------------------------------------------------------------
int* ML_GenerateTruthTable(int dim, LogicFunc_t func, int *numRows)
{
    *numRows   = 1 << dim;          // 2^dim filas
    int cols   = dim + 1;
    int total  = (*numRows) * cols;

    int *table = (int*) malloc(total * sizeof(int));
    if (!table) 
    {
        Serial.println("[ML] ERROR: Sin memoria para tabla de verdad.");
        return nullptr;
    }

    // Rellenar las columnas de entrada con el patrón binario
    // Columna 0: periodo = 2^(dim-1), columna 1: 2^(dim-2), ...
    for (int col = 0; col < dim; col++) 
    {
        int period = 1 << (dim - col - 1);
        for (int row = 0; row < *numRows; row++) 
        {
            table[row * cols + col] = (row / period) % 2;
        }
    }

    // Calcular la salida AND u OR para cada fila
    for (int row = 0; row < *numRows; row++) 
    {
        int result = (func == LOGIC_AND) ? 1 : 0;
        for (int col = 0; col < dim; col++) 
        {
            int val = table[row * cols + col];
            if (func == LOGIC_AND) result = result & val;
            else                   result = result | val;
        }
        table[row * cols + dim] = result;
    }

    return table;
}

// ----------------------------------------------------------------
//  ML_GenerateCustomTable                              ← NUEVO
//  Descripción : Genera la tabla de verdad con las entradas
//                calculadas automáticamente (patrón binario) y
//                solicita al usuario por Serial la salida deseada
//                (0 o 1) para cada combinación de entradas.
//                La tabla tiene 2^dim filas y (dim+1) columnas.
//                La última columna es la salida ingresada.
//  Parámetros  : dim     — número de variables (= pots)
//                numRows — salida: número de filas generadas
//  Retorna     : Puntero al arreglo aplanado (filas × cols)
//                El llamador debe liberar con free().
// ----------------------------------------------------------------
int* ML_GenerateCustomTable(int dim, int *numRows)
{
    *numRows  = 1 << dim;           // 2^dim filas
    int cols  = dim + 1;
    int total = (*numRows) * cols;

    int *table = (int*) malloc(total * sizeof(int));
    if (!table) 
    {
        Serial.println("[ML] ERROR: Sin memoria para tabla personalizada.");
        return nullptr;
    }

    // ── Llenar columnas de entrada con el patrón binario estándar ──
    for (int col = 0; col < dim; col++) 
    {
        int period = 1 << (dim - col - 1);
        for (int row = 0; row < *numRows; row++) 
        {
            table[row * cols + col] = (row / period) % 2;
        }
    }

    // Solicitar la salida deseada para cada fila
    Serial.println("[HMI] Ingresa la salida deseada (0 o 1) para cada combinacion de entradas.");
    Serial.println("[HMI] Escribe el valor y presiona ENTER.");
    PRINT_Separator();

    // Encabezado
    Serial.print("  ");
    for (int i = 0; i < dim; i++) Serial.printf("X%d  ", i + 1);
    Serial.println("→  Y (escribe 0 o 1)");
    PRINT_Separator();

    for (int row = 0; row < *numRows; row++) 
    {
        // Mostrar la combinación de entradas de esta fila
        Serial.print("  [");
        for (int col = 0; col < dim; col++) 
        {
            Serial.print(table[row * cols + col]);
            if (col < dim - 1) Serial.print(",");
        }
        Serial.print("]  →  ");

        // Leer la salida del usuario con validación
        int y = -1;
        while (y != 0 && y != 1) 
        {
            // Esperar datos en el buffer
            while (!Serial.available()) 
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            y = Serial.parseInt();
            // Vaciar el resto del buffer (CR, LF, espacios extra)
            while (Serial.available()) Serial.read();

            if (y != 0 && y != 1) 
            {
                Serial.print("[HMI] Solo 0 o 1, intenta de nuevo: ");
            }
        }

        table[row * cols + dim] = y;
        Serial.printf("  (guardado: %d)\n", y);
    }

    PRINT_Separator();
    Serial.println("[ML] Tabla personalizada capturada correctamente.");
    return table;
}

// ----------------------------------------------------------------
//  ML_PrintTruthTable
//  Descripción : Muestra la tabla de verdad formateada en consola.
//  Parámetros  : table   — tabla generada por ML_GenerateTruthTable
//                numRows — número de filas
//                dim     — número de variables de entrada
// ----------------------------------------------------------------
void ML_PrintTruthTable(const int *table, int numRows, int dim)
{
    PRINT_Separator();
    Serial.print("[ML] Tabla de verdad  |  ");
    for (int i = 0; i < dim; i++) Serial.printf("X%d  ", i + 1);
    Serial.println("|  Y");
    PRINT_Separator();

    int cols = dim + 1;
    for (int r = 0; r < numRows; r++) 
    {
        Serial.print("                      ");
        for (int c = 0; c < dim; c++) {
            Serial.printf("%d   ", table[r * cols + c]);
        }
        Serial.printf("|  %d\n", table[r * cols + dim]);
    }
    PRINT_Separator();
}

/* ================================================================
 *  ML_Train
 *  Descripción : Entrena el perceptrón con el algoritmo LMS.
 *                Este es el CORAZÓN del módulo ML.
 *
 *  ALGORITMO LMS — PASO A PASO:
 *  1. Para cada época (iteración completa sobre todos los ejemplos):
 *     a. Para cada ejemplo (fila de la tabla):
 *        i.  Calcular la suma ponderada: g(x) = suma(wk·xk) + w0
 *        ii. Aplicar hardlim: salida = 1 si g>=0.5, else 0
 *        iii.Calcular error: e = y_real - salida
 *        iv. Actualizar pesos: wk = wk + n·e·xk  (para k=1..N)
 *                              w0 = w0 + n·e      (bias)
 *     b. Verificar convergencia: si TODOS los |e| < umbral -> terminar
 *  2. Si se alcanza el máximo de épocas → terminar igualmente
 *
 *  Parámetros:
 *    p       -> puntero al modelo (ya inicializado con ML_Init)
 *    table   -> tabla de verdad completa (entradas + salidas deseadas)
 *    numRows -> número de ejemplos de entrenamiento
 * ================================================================ */
void ML_Train(Perceptron_t *p, const int *table, int numRows)
{
    int    dim   = p->dim;
    int    cols  = dim + 1;
    float *error = (float*) malloc(numRows * sizeof(float));

    // Inicializar error grande para entrar al ciclo while
    for (int i = 0; i < numRows; i++) error[i] = 1.0f;

    Serial.println("[ML] Iniciando entrenamiento LMS...");
    unsigned long t0 = millis();

    int  epoch     = 0;
    bool converged = false;

    while (!converged && epoch < BSP_LMS_MAX_EPOCHS) 
    {

        // Una época: procesar cada ejemplo
        for (int i = 0; i < numRows; i++) 
        {

            // Calcular salida: g(x) = sum(wk · xk) + w0
            float g = p->weights[dim];              // bias
            for (int k = 0; k < dim; k++) {
                g += p->weights[k] * table[i * cols + k];
            }

            int out       = hardlim(g);
            int y_real    = table[i * cols + dim];
            error[i]      = (float)(y_real - out);

            // Actualización de pesos: wk = wk + n · e · xk
            for (int k = 0; k < dim; k++) {
                p->weights[k] += BSP_LMS_ETA * error[i] * table[i * cols + k];
            }
            // Actualización del bias: w0 = w0 + n · e
            p->weights[dim] += BSP_LMS_ETA * error[i];
        }

        epoch++;

        // Verificar convergencia: |e[i]| < MinError para todos
        converged = true;
        for (int i = 0; i < numRows; i++) 
        {
            if (fabsf(error[i]) >= BSP_LMS_MIN_ERROR) 
            {
                converged = false;
                break;
            }
        }

        // Ceder CPU al scheduler de FreeRTOS cada 50 épocas
        // para no bloquear otras tareas de mayor prioridad.
        if (epoch % 50 == 0) {
            Serial.printf("[ML] Epoca %d ...\n", epoch);
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }

    unsigned long elapsed = millis() - t0;
    p->epochs  = epoch;
    p->trained = true;

    free(error);

    // Reporte final
    PRINT_Separator();
    if (converged) {
        Serial.printf("[ML] Convergencia en %d epocas (%lu ms)\n", epoch, elapsed);
    } else {
        Serial.printf("[ML] Parado en maximo de epocas (%d) en %lu ms\n", epoch, elapsed);
        Serial.println("[ML] NOTA: Puede que la funcion no sea linealmente separable.");
    }
    Serial.print("[ML] Pesos finales: ");
    for (int i = 0; i < dim; i++) {
        Serial.printf("w%d=%.4f  ", i + 1, p->weights[i]);
    }
    Serial.printf("bias=%.4f\n", p->weights[dim]);
    PRINT_Separator();
}

// ----------------------------------------------------------------
//  ML_Predict
//  Descripción : Inferencia: calcula la salida del perceptrón
//                dado un vector de entrada binarizado.
//  Parámetros  : p - modelo entrenado
//                x - arreglo de dim entradas (0 o 1)
//  Retorna     : 0 o 1
// ----------------------------------------------------------------
int ML_Predict(const Perceptron_t *p, const int *x)
{
    float g = p->weights[p->dim];   // bias
    for (int k = 0; k < p->dim; k++) {
        g += p->weights[k] * x[k];
    }
    return hardlim(g);
}

// ----------------------------------------------------------------
//  ML_Verify
//  Descripción : Prueba el modelo entrenado con todos los ejemplos
//                de la tabla de verdad e imprime la precisión.
//  Parámetros  : p       - modelo entrenado
//                table   - tabla de verdad
//                numRows - número de filas de la tabla
// ----------------------------------------------------------------
void ML_Verify(const Perceptron_t *p, const int *table, int numRows)
{
    int  dim  = p->dim;
    int  cols = dim + 1;
    int  ok   = 0;
    int  x[BSP_MAX_POTS];

    Serial.println("[ML] Verificacion del modelo:");
    PRINT_Separator();
    Serial.print("  Entrada");
    for (int i = 0; i < dim - 7; i++) Serial.print(" ");
    Serial.println("  | Y_real | Y_pred | OK?");
    PRINT_Separator();

    for (int i = 0; i < numRows; i++) {
        for (int k = 0; k < dim; k++) x[k] = table[i * cols + k];

        int y_real = table[i * cols + dim];
        int y_pred = ML_Predict(p, x);
        bool correct = (y_real == y_pred);
        if (correct) ok++;

        Serial.print("  [");
        for (int k = 0; k < dim; k++) {
            Serial.print(x[k]);
            if (k < dim - 1) Serial.print(",");
        }
        Serial.printf("]   |   %d    |   %d    | %s\n", y_real, y_pred, correct ? "OK" : "FALLO");
    }

    PRINT_Separator();
    Serial.printf("[ML] Precision: %d / %d  (%.0f%%)\n", ok, numRows, 100.0f * ok / numRows);
    PRINT_Separator();
}

// ----------------------------------------------------------------
//  ML_Free
//  Descripción : Libera la memoria dinámica del modelo.
//                Llamar al reinicializar o apagar el sistema.
//  Parámetros  : p — puntero al modelo
// ----------------------------------------------------------------
void ML_Free(Perceptron_t *p)
{
    if (p->weights != nullptr) 
    {
        free(p->weights);
        p->weights = nullptr;
    }
    p->trained = false;
    p->dim     = 0;
}
