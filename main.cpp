#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <chrono>

using namespace std;

class DispositivoComunicacion {
protected:
	std::string id;
	std::string tipo;
public:
	DispositivoComunicacion(std::string _id, std::string _tipo)
		: id(_id), tipo(_tipo) {}
	
	// Destructor virtual: indispensable para liberaciï¿½n polimï¿½rfica en el Heap
	virtual ~DispositivoComunicacion() = default;
	
	// Mï¿½todo virtual puro: define la interfaz obligatoria
	virtual void transmitir() = 0;
	
	std::string getId() const { return id; }
	std::string getTipo() const { return tipo; }
};

// SUBCLASES CONCRETAS CON SOBRESCRITURA EXPLï¿½CITA (override)
class RouterWiFi : public DispositivoComunicacion {
private:
	int canalesActivos;
public:
	RouterWiFi(std::string _id, int _canales)
		: DispositivoComunicacion(_id, "Router WiFi 6"), canalesActivos(_canales) {}
	
	void transmitir() override {
		// Lï¿½gica de transmisiï¿½n especï¿½fica sobre la banda de 5 GHz
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
};

class GatewayLoRa : public DispositivoComunicacion {
private:
	float frecuenciaMHz;
public:
	GatewayLoRa(std::string _id, float _freq)
		: DispositivoComunicacion(_id, "Gateway LoRaWAN"), frecuenciaMHz(_freq) {}
	
	void transmitir() override {
		// Lï¿½gica de transmisiï¿½n de larga distancia y bajo consumo
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
};
// MUTEX GLOBAL Y REGISTRO CONCURRENTE PROTEGIDO (RAII)
std::mutex mtx_consola; // Protege la salida estï¿½ndar compartida

void registrarEventoSeguro(const std::string& mensaje) {
	// RAII: Bloquea mtx_consola al crearse y lo desbloquea automï¿½ticamente al salir
	std::lock_guard<std::mutex> lock(mtx_consola);
	std::cout << "[HILO " << std::this_thread::get_id() << "] " 
		<< mensaje << std::endl;
}

// FUNCIï¿½N ENVOLVENTE CON GUARDA Y CONTROL INTERNO DE EXCEPCIONES
void tareaTransmisionHilo(DispositivoComunicacion* dev) {
	try {
		// Guarda defensiva contra punteros nulos
		if (dev == nullptr) {
			throw std::invalid_argument("ERROR: Puntero nulo pasado al hilo.");
		}
		
		// Invocaciï¿½n polimï¿½rfica del mï¿½todo virtual
		dev->transmitir();
		registrarEventoSeguro("Transmisiï¿½n exitosa del dispositivo: " + dev->getId());
	}
	catch (const std::exception& e) {
		// Captura y aislamiento de la excepciï¿½n dentro del hilo
		registrarEventoSeguro(std::string("EXCEPCIIÓN EN HILO: ") + e.what());
	}
}


int main (int argc, char *argv[]) {

	// 1. Vector heterogï¿½neo de dispositivos en el Heap
	std::vector<DispositivoComunicacion*> red;
	red.push_back(new RouterWiFi("RTR-01", 12));
	red.push_back(new GatewayLoRa("GW-868", 868.1f));
	red.push_back(nullptr); // Provocaciï¿½n intencional para probar la guarda defensiva
	
	// 2. Vector de hilos independientes
	std::vector<std::thread> hilos;
	for (DispositivoComunicacion* dev : red) {
		hilos.push_back(std::thread(tareaTransmisionHilo, dev));
	}
	
	// 3. Sincronizaciï¿½n obligatoria de hilos
	for (auto& t : hilos) {
		if (t.joinable()) {
			t.join();
		}
	}
	
	// 4. Liberaciï¿½n limpia de memoria dinamica
	for (DispositivoComunicacion* dev : red) {
		delete dev;
	}
	red.clear();
	
	std::cout << "Sistema finalizado correctamente." << std::endl;
		
	return 0;
}
