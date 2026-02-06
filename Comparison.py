import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("throughput_vs_ports.csv")

plt.plot(df["ports"], df["avg_packets_per_slot"],
         marker='o', linewidth=2)

plt.xlabel("Number of Input Ports")
plt.ylabel("Average Throughput (packets/slot)")
plt.title("Average Throughput vs Number of Ports (APSARA)")
plt.xticks(df["ports"])
plt.grid(True)

plt.tight_layout()
plt.savefig("throughput_packets_per_slot.png", dpi=300)
plt.show()

plt.figure(figsize=(8, 5))
plt.plot(df["ports"], df["avg_throughput_percent"],
         marker='o', linewidth=2, color='green')

plt.xlabel("Number of Input Ports")
plt.ylabel("Average Throughput (%)")
plt.title("Average Throughput (%) vs Number of Ports (APSARA)")
plt.xticks(df["ports"])
plt.grid(True)

plt.tight_layout()
plt.savefig("throughput_percent.png", dpi=300)
plt.show()

