import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np

def plot_info(full_data, x_axis, y_axis):
    kys = full_data.keys()
    colors = cm.rainbow(np.linspace(0,1, len(kys)))
    clr = 0
    for i in kys:
        plt.scatter(full_data[i][x_axis], full_data[i][y_axis], label=i,
                    color=colors[clr])
        clr = clr + 1
    plt.xlabel(x_axis)
    plt.ylabel(y_axis)
    plt.legend()
    #plt.show()
    plt.savefig(y_axis.strip() + '.png')
    plt.close()
    
def main():
    df = pd.read_csv("data.csv");
    print(df)
    column_names = list(df)
    methods = pd.unique(df[column_names[0]])
    noise_levels = pd.unique(df[column_names[1]])
    print("column names")
    print(column_names)
    print("methods")
    print(methods)
    print("noise levels")
    print(noise_levels)
    statistical_info = {}
    for i in methods:
        new_info = pd.DataFrame(columns=column_names[1:(len(column_names))])
        for j in noise_levels:
            stat = df[df[column_names[0]] == i][(df[column_names[1]] > j - 0.1) & (df[column_names[1]] < j + 0.1)].mean()
            count_info = df[df[column_names[0]] == i][(df[column_names[1]] > j - 0.1) & (df[column_names[1]] < j + 0.1)].shape
            print("Count")
            print("method: " + i + " noise level: " + str(j))
            print(count_info)
            #print(stat)
            new_info = new_info.append(stat, ignore_index=True)
        print(new_info)
        statistical_info[i] = new_info
    kys = statistical_info.keys()
    for i in kys:
        print("**************")
        print(i)
        print(statistical_info[i])
    
    plot_info(statistical_info, column_names[1], column_names[2])
    plot_info(statistical_info, column_names[1], column_names[3])
    plot_info(statistical_info, column_names[1], column_names[4])
if __name__ == "__main__":
    main()

