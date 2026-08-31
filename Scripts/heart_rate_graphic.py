import matplotlib.pyplot as plt

date_str = input("Put the data hear:");

value = [int(x) for x in date_str.split(',')]

plt.figure(figsize=(12, 5))
plt.plot(value, color='#1f77b4', linewidth=1.5)

plt.title('Puls Senzor Value', fontsize=14)
plt.xlabel('Esantion', fontsize=12)
plt.ylabel('Values', fontsize=12)
plt.grid(True, linestyle='--', alpha=0.7)
plt.tight_layout()

plt.savefig('Graph_HeartRate.png', bbox_inches='tight')
plt.show()