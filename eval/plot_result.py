import pandas as pd
import matplotlib.pyplot as plt
import sys

try:
    dirname = sys.argv[1]
except:
    print("Usage: python plot_result.py <dirname>")
    sys.exit()


# Read the data from the csv file
pub_df = pd.read_csv(dirname + '/pub.csv', index_col=2, names=['data_index', 'time_ns'])
sub_df = pd.read_csv(dirname + '/sub.csv', index_col=2, names=['data_index', 'time_ns'])

# Calculate the round trip time
diff_time = sub_df['time_ns'] - pub_df['time_ns']
#print(diff_time.tail())

# Show the plot
plt.figure(figsize=(8, 4), tight_layout=True)
ax = plt.subplot(1,1,1)
ax.plot(diff_time*1e-6, '.', label='Round trip time')
ax.set_xlabel('Data_index')
ax.set_ylabel('Round trip time [ms]')
ax.set_xlim([-10, diff_time.index.max()+11])
ax.set_ylim([0, diff_time.max()*1e-6+1])
ax.set_title(dirname)

#if 
#ax.legend(loc='upper left')
ax.grid()

# pngで保存
plt.savefig(dirname + '/result.png', format="png", dpi=300)
#plt.show()

print("Average round trip time: {} ms".format(diff_time.mean()*1e-6))
print("Maximum round trip time: {} ms".format(diff_time.max()*1e-6))
print("Minimum round trip time: {} ms".format(diff_time.min()*1e-6))
print("Standard deviation: {} ms".format(diff_time.std()*1e-6)) # 標準偏差
print("Number of null: {}".format(diff_time.isnull().sum())) # 帰ってこなかったメッセージの数
