import pandas as pd
import matplotlib.pyplot as plt
import os
import re
import numpy as np
from datetime import datetime
import argparse

def read_data(directorio):
    resultados = []
    for archivo in os.listdir(directorio):
        
        # Verificamos que el nombre del archivo cumpla con el patrón esperado
        match = re.match(r'^threads_(\d+)_(\d+)_(\d+)_(\d+)\.csv$', archivo)
        if match:
            ruta_completa = os.path.join(directorio, archivo)
            df = pd.read_csv(ruta_completa)
            num_hilos = int(match.group(1))
            metodo = match.group(4)
            duracion_media = df['Duracion'].mean()
            resultados.append((num_hilos, metodo, duracion_media))
            print(f"Archivo leído: {archivo} - Método: {metodo} - Duración Media: {duracion_media}")
    
    # Convertimos la lista de resultados en un DataFrame y lo ordenamos
    resultados_df = pd.DataFrame(resultados, columns=['Hilos', 'Método', 'Duración Media'])
    return resultados_df.sort_values(by=['Hilos', 'Método'])

def graphics(df, output_dir=None):
    plt.figure()
    
    # Obtenemos valores máximos y mínimos para los ejes
    max_hilos = df['Hilos'].max()
    min_hilos = df['Hilos'].min()
    max_duracion = df['Duración Media'].max()
    min_duracion = df['Duración Media'].min()

    # Dibujamos una línea para cada método encontrado en el DataFrame
    for metodo in df['Método'].unique():
        df_metodo = df[df['Método'] == metodo]
        plt.plot(df_metodo['Hilos'], df_metodo['Duración Media'], label=f'Método {metodo}', marker='o')

    # Configuramos títulos y etiquetas del gráfico
    plt.title('Comparación entre los Métodos Utilizados')
    plt.xlabel('Número de Hilos')
    plt.ylabel('Duración Media (ns)')
    plt.legend()
    plt.grid(True)

    # Ajustamos las marcas de los ejes
    num_ticks = 10
    plt.xticks(ticks=np.linspace(min_hilos, max_hilos, num_ticks, dtype=int))
    plt.yticks(ticks=np.linspace(min_duracion, max_duracion, num_ticks))

    # Decidimos si mostrar o guardar la gráfica en función del directorio de salida
    if output_dir is None:
        plt.show()
    else:
        if not os.path.exists(output_dir):
            print(f"Error: El directorio {output_dir} no existe. Por favor crea el directorio antes de correr el script.")
            return
        fecha_actual = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        nombre_archivo = f"Resultado_{fecha_actual}.png"
        path_completo = os.path.join(output_dir, nombre_archivo)
        plt.savefig(path_completo)
        plt.close()
        print(f"Gráfica guardada como: {path_completo}")

if __name__ == "__main__":
    
    # Configuramos el parser de argumentos
    parser = argparse.ArgumentParser(description='Procesa archivos CSV para generar gráficas.')
    parser.add_argument('directorio', type=str, help='Directorio donde se encuentran los archivos CSV.')
    parser.add_argument('--output_dir', '-o', type=str, default=None, help='Directorio para guardar los resultados')
    args = parser.parse_args()

    # Leemos los datos y generamos la gráfica
    resultados_df = read_data(args.directorio)
    graphics(resultados_df, args.output_dir)