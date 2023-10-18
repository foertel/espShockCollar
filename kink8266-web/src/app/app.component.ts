import { Component } from '@angular/core';
import { SocketService } from './services/socket.service';

@Component({
	selector: 'app-root',
	templateUrl: './app.component.html',
	styleUrls: ['./app.component.scss']
})
export class AppComponent {
	title = 'kink8266-web';
	activeGames: Array<any> = [ ];
	availableCollars: Array<any> = [ ];

	constructor(public socketService: SocketService) {
		socketService.on('gamesUpdated', (data => {
			console.log(data);
			this.activeGames = [ ];
			for(let d in data){
				let tmp = data[d];
				if(tmp.playerCollars?.length < tmp.playerCount){
					for(let i = 0; i < tmp.playerCount; i++){
						if(!tmp.playerCollars[i])
							tmp.playerCollars[i] = null;
					}
				}
				this.activeGames.push(tmp);
			}
		}))

		socketService.on('collarsUpdated', (data => {
			this.availableCollars = data;
		}));

		socketService.on('connect', () => {
			this.sendInitData();
		})
		this.sendInitData();
	}

	sendInitData(){
	}

	setCollarGame(gameName: string, $event: any, playerNumber: number) {
		let collarId = parseInt($event.target.value);

		this.socketService.emit('assignCollar', collarId, gameName, playerNumber);
	}

	saveCollarChanges(collar: any) {
		this.socketService.emit('editCollar', collar);
	}

	removeCollar (collar: any) {
		this.socketService.emit('removeCollar', collar);
	}

	linkCollar(collar: any) {
		this.socketService.emit('linkCollar', collar);
	}

	vibrateCollar(collar: any) {
		this.socketService.emit('vibrateCollar', collar);
	}

	beepCollar(collar: any) {
		this.socketService.emit('beepCollar', collar);
	}
}
