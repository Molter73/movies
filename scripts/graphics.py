import pandas as pd
import matplotlib.pyplot as plt
import os
import re
import numpy as np
import argparse

METHODS = [
    'Global',
    'Fila',
    'Asiento',
]


def read_data(directorio, exclude_method):
    resultados = pd.DataFrame(columns=[
        'Hilos',
        'Método',
        'Duración Media',
        'Columnas',
        'Filas',
        'Carga',
    ])
    for archivo in os.listdir(directorio):
        # Verificamos que el nombre del archivo cumpla con el patrón esperado
        match = re.match(
            r'^threads_(\d+)_(\d+)_(\d+)_(\d+)(_loaded)?\.csv$', archivo)
        if match:
            method = int(match.group(4))
            if method in exclude_method:
                continue

            ruta_completa = os.path.join(directorio, archivo)
            df = pd.read_csv(ruta_completa)
            resultados.loc[len(resultados.index)] = {
                'Hilos': int(match.group(1)),
                'Filas': int(match.group(2)),
                'Columnas': int(match.group(3)),
                'Duración Media': df['Duracion'].mean(),
                'Método': METHODS[method],
                'Carga': match.group(5) is not None,
            }

    return resultados.sort_values(by=['Hilos', 'Método'])


def graphic(df, rows, cols, load, output_dir=None):
    plt.figure()

    # Obtenemos valores máximos y mínimos para los ejes
    max_hilos = df['Hilos'].max()
    min_hilos = df['Hilos'].min()
    max_duracion = df['Duración Media'].max()
    min_duracion = df['Duración Media'].min()

    # Dibujamos una línea para cada método encontrado en el DataFrame
    for metodo in df['Método'].unique():
        df_metodo = df[df['Método'] == metodo]
        plt.plot(df_metodo['Hilos'], df_metodo['Duración Media'],
                 label=f'Método {metodo}', marker='o')

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

    # Decidimos si mostrar o guardar la gráfica en función
    # del directorio de salida
    if output_dir is None:
        plt.show()
    else:
        path_completo = os.path.join(
            output_dir,
            f"Resultado_{rows}_{cols}{'_loaded' if load else ''}.png"
        )
        plt.savefig(path_completo)
        plt.close()


def graphics(df, output_dir=None):
    for i, j in df.groupby(['Filas', 'Columnas', 'Carga']):
        graphic(j, i[0], i[1], i[2], output_dir)
    return


if __name__ == "__main__":
    # Configuramos el parser de argumentos
    parser = argparse.ArgumentParser(
        description='Procesa archivos CSV para generar gráficas.')
    parser.add_argument('directorio', type=str,
                        help='Directorio donde se encuentran los archivos CSV.'
                        )
    parser.add_argument('--output_dir', '-o', type=str, default=None,
                        help='Directorio para guardar los resultados')
    parser.add_argument('--exclude-method', '-e', action='append', type=int,
                        default=[], help='Excluir métodos del gráfico final')
    args = parser.parse_args()

    # Leemos los datos y generamos la gráfica
    resultados_df = read_data(args.directorio, args.exclude_method)
    graphics(resultados_df, args.output_dir)
