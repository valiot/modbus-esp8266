/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "ModbusIP_ESP8266.h"

//ModbusIP::ModbusIP() {
//
//}

void ModbusIP::begin() {
	WiFiServer::begin();
}

void ModbusIP::task() {
	for (uint8_t n = 0; n < TCP_MAX_CLIENTS; n++) {
		if (!client[n] || !client[n].connected()) {
			client[n] = available();
		}

	int raw_len = 0;
	
    	if (client[n]) {
		if (client[n].connected()) {
		    for (int x = 0; x < 300; x++) { // Time to have data available
				if (client[n].available()) {
					while (client[n].available() > raw_len) {  //Computes data length
						raw_len = client[n].available();
						delay(1);
					}
					break;
				}
				delay(10);				
			}
		}
				
		if (raw_len > 7) {
			for (int i=0; i<7; i++)	_MBAP[i] = client[n].read(); //Get MBAP

			_len = _MBAP[4] << 8 | _MBAP[5];
			_len--; // Do not count with last byte from MBAP
			if (_MBAP[2] !=0 || _MBAP[3] !=0) return;   //Not a MODBUSIP packet
			if (_len > MODBUSIP_MAXFRAME) return;      //Length is over MODBUSIP_MAXFRAME
			_frame = (byte*) malloc(_len);
			
			raw_len = raw_len - 7;
			for (int i=0; i< raw_len; i++)	_frame[i] = client[n].read(); //Get Modbus PDU
			
			this->receivePDU(_frame);
			client[n].flush();

			if (_reply != MB_REPLY_OFF) {
			    //MBAP
				_MBAP[4] = (_len+1) >> 8;     //_len+1 for last byte from MBAP
				_MBAP[5] = (_len+1) & 0x00FF;
				
				size_t send_len = (unsigned int)_len + 7;
				uint8_t sbuf[send_len];
				
				for (int i=0; i<7; i++)	    sbuf[i] = _MBAP[i];
				for (int i=0; i<_len; i++)	sbuf[i+7] = _frame[i];

				client[n].write(sbuf, send_len);
			}
		#ifndef TCP_KEEP_ALIVE
			client[n].stop();
		#endif
			free(_frame);
			_len = 0;
		}
	}
}
}