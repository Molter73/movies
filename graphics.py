import pandas as pd
import matplotlib.pyplot as plt
import os
import re
import sys
import numpy as np
from datetime import datetime
import argparse

def graphics(directorio):
    resultados = []

    for archivo in os.listdir(directorio):
        match = re.match(r'^threads_(\d+)_(\d+)_(\d+)_(\d+)\.csv$', archivo)
        if match:
            ruta_completa = os.path.join(directorio, archivo)
            df = pd.read_csv(ruta_completa)

            # Extraemos Hilos y Método
            num_hilos = int(match.group(1))
            metodo = match.group(4)

            # Calculamos duración media
            duracion_media = df['Duracion'].mean()

            # Guardamos resultados
            resultados.append((num_hilos, metodo, duracion_media))
            print(f"Archivo leído: {archivo} - Método: {metodo} - Duración Media: {duracion_media}")

    # Creamos DataFrame y ordenamos
    resultados_df = pd.DataFrame(resultados, columns=['Hilos', 'Método', 'Duración Media'])
    resultados_df = resultados_df.sort_values(by=['Hilos', 'Método'])

    # Graficamos
    plt.figure()
    max_hilos = resultados_df['Hilos'].max()
    min_hilos = resultados_df['Hilos'].min()
    max_duracion = resultados_df['Duración Media'].max()
    min_duracion = resultados_df['Duración Media'].min()

    for metodo in resultados_df['Método'].unique():
        df_metodo = resultados_df[resultados_df['Método'] == metodo]
        plt.plot(df_metodo['Hilos'], df_metodo['Duración Media'], label=f'Método {metodo}', marker='o')

    plt.title('Comparación entre los Métodos Utilizados')
    plt.xlabel('Número de Hilos')
    plt.ylabel('Duración Media (ns)')
    plt.legend()
    plt.grid(True)

    num_ticks = 10
    plt.xticks(ticks=np.linspace(min_hilos, max_hilos, num_ticks, dtype=int))
    plt.yticks(ticks=np.linspace(min_duracion, max_duracion, num_ticks))

    # Directorio para guardar resultados
    output_dir = "results"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Nombre del archivo con fecha
    fecha_actual = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    nombre_archivo = f"Resultado_{fecha_actual}.png"
    path_completo = os.path.join(output_dir, nombre_archivo)

    # Guardamos la gráfica
    plt.savefig(path_completo)
    plt.close()

    print(f"Gráfica guardada como: {path_completo}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Procesa archivos CSV para generar gráficas.')
    parser.add_argument('directorio', type=str, help='Directorio donde se encuentran los archivos CSV.')
    args = parser.parse_args()
    graphics(args.directorio)
