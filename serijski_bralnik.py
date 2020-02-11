import serial
import serial.tools.list_ports
from time import time
from datetime import datetime

# nekaj nastavitev 
n_meritev = 1  # koliko meritev pricakujemo iz Arduina
meritve_dat = 'co2_senzor_kalibracija_2.csv' # v katero datoteko bomo shranjevali meritve

def branje_in_shranjevanje_podatkov(arduino_port):
	# prikljucimo se na serijski vmesnik preko katerega bomo prejemali in posiljali podatke
	arduino_serijski = serial.Serial(arduino_port.device, 9600, timeout=10) # timeout - koliko casa najvec cakamo
	
	txt = open(meritve_dat, 'a')

	# neskoncna zanka za branje podatkov
	while True:
		# pocakamo in preberemo eno celo poslano vrstico
		podatki = arduino_serijski.readline()
		# prebrane podatke iz binarne oblike pretvorimo v string
		podatki = str(podatki, 'utf-8')
		# odstranimo nezazelene podatke za konec vrstice
		podatki = podatki.replace('\r', '')
		podatki = podatki.replace('\n', '')

		# preverimo ali so nasi podatki sploh smiselni in imajo zahtevano stevilo meritev locenih z vejico (,)
		if len(podatki.split(',')) != n_meritev:
			# nekaj ne stima, pocakajmo na nove podatke
			continue

		# dopolnimo podatke s casom meritve
		cas_sedaj = time() # v sekundah
		datum_sedaj = datetime.now()
		podatki_dopolnjeni = str(datum_sedaj) + ',' + str(cas_sedaj) + ',' + podatki

		# izpise na zaslon
		print(podatki_dopolnjeni)
		
		# odpremo se tekstovno datoteko kamor bomo zapisovali podatke
		txt = open(meritve_dat, 'a') # 'a' pomeni dodaj podatke na koncu datoteke
		# shranimo v izbrano datoteko
		txt.write(podatki_dopolnjeni + '\r\n')
		# in jo zapremo, da se shranijo vrednosti
		txt.close()

	# po koncanem branju zapremo vmesnik
	arduino_serijski.close()


if __name__ == '__main__':

	# pridobi seznam vseh COM vrat na pc-ju
	port_vsi = list(serial.tools.list_ports.comports())

	if len(port_vsi) == 0:
		prin('Nisem nasel priemrnih vrat')
		raise SystemExit

	# izberimo pravi port, ki naj bi kazal na Arduino (se poveze na prvega zaznanega)
	for port in port_vsi:
		if 'ttyUSB' in str(port):
			arduino_port = port
			print('Izbran:', port)
			break

	# poglej ce smo ga sploh nasli
	if 'arduino_port' not in locals():
		prin('Arduino ni prikljucen')
		raise SystemExit

	# po uspesnem dolocanju porta lahko zacnemo izvajat nase branje in shranjevanje podatkov
	branje_in_shranjevanje_podatkov(arduino_port)
