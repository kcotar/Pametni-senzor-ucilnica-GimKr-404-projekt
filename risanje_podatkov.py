import matplotlib.pyplot as plt
import pandas as pd

podatki = pd.read_csv('co2_senzor_kalibracija_2.csv', sep=',', header= None)
datum = pd.to_datetime(podatki[0])

print(podatki)

for col in podatki.columns[2:]:
	plt.plot(datum, podatki[col])
	plt.xlabel('Cas in datum')
	plt.ylabel('Vrednost meritve')
	plt.tight_layout()
	plt.show()
	plt.close()
