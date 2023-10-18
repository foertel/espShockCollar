import { Injectable } from '@angular/core';

import { environment } from 'src/environments/environment.development';

import * as io from "socket.io-client";

@Injectable({
	providedIn: 'root'
})
export class SocketService {
	private socket: SocketIOClient.Socket;
	constructor() {
		this.socket = io(environment.schockMasterUrl, {
			transports: ['websocket']
		});

		this.socket.on('connect', () => {
			this.emit('joinFrontend');
		})
	}

	public on(eventName: string, fn: (...args: any) => any){
		console.log("[SocketService::on]: "+eventName);
		this.socket.on(eventName, fn);
	}

	public emit(eventName: string, ...data: Array<any>){
		console.log("[SocketService::emit]: "+eventName, JSON.stringify(data));
		this.socket.emit(eventName, ...data);
	}
}