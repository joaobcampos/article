import pandas as pd
import matplotlib.pyplot as plt

def plot_info(full_data, x_axis, y_axis):
    kys = full_data.keys()
    clr = 0;
    for i in kys:
        plt.scatter(full_data[i][x_axix], full_data[i][y_axis], label=i, c=clr)
        clr++
    plt.show()

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
            #print(stat)
            new_info = new_info.append(stat, ignore_index=True)
        print(new_info)
        statistical_info[i] = new_info
    kys = statistical_info.keys()
    for i in kys:
        print("**************")
        print(i)
        print(statistical_info[i])
        
if __name__ == "__main__":
    main()

