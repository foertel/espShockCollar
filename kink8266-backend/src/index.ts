import "reflect-metadata";

import * as sourceMapSupport from 'source-map-support';
sourceMapSupport.install();


import { createServer } from "http";
const app = createServer(handler);

import * as io from "socket.io";
import { DataSource, DataSourceOptions, createConnection } from 'typeorm';

import { root } from './path';

const serverSock = io(app)

var fs = require('fs');
const port = 8080;
console.log("listening on port " + port);
app.listen(port);

function handler(req, res) {
	console.log("http req.");
	fs.readFile(__dirname + '/index.html',
		function (err, data) {
			if (err) {
				res.writeHead(500);
				return res.end('Error loading index.html');
			}

			res.writeHead(200);
			res.end(data);
		});
}

import * as entities from "./entities";
import { randomUUID } from "crypto";

const AppDataSource = new DataSource({


	type: "sqlite",
	database: `${root}/data/line.sqlite`,
	entities: [entities.Collar],
	logging: true,
	synchronize: true,

	subscribers: [],
	migrations: [],
});


async function main() {
	await AppDataSource.initialize();
	await AppDataSource.synchronize();
	console.log(AppDataSource.isInitialized);
}

main();

interface GameData {
	name: string;
	humanName: string;
	playerCount: number;

	socket?: io.Socket;
	playerCollars: Array<entities.Collar>
}

interface GameSchockData {
	name: string; // unique identifier as in GameData
	player: number; // number from zero to infinity, the player to be schocked
	intensity: IntensityLevels;
}

type IntensityLevels = "high" | "medium" | "low";

interface CollarRegisterData {
	channel: number;
	receiverId: number;
}

const connectedGames: Map<string, GameData> = new Map();

const FRONTEND_NAMESPACE = "frontend";
const SCHOCKER_NAMESPACE = "schocker";


serverSock.on('connection', function (socket) {
	console.log("new connection .. ", socket.id)
	socket.emit('helo');
	socket.on('my other event', function (data) {
		console.log(data);
	});

	socket.on('joinSchocker', () => {
		console.log("joined schocki");
		socket.join(SCHOCKER_NAMESPACE);

		socket.on('newCollar', async (collarData: CollarRegisterData) => {
			let existing = await entities.Collar.findOneBy({
				receiverId: collarData.receiverId
			});

			if(existing)
				return;
			
			let collar = new entities.Collar();
			collar.maximumIntensity = 10;
			collar.receiverId		= collarData.receiverId;
			collar.channel			= collarData.channel;
			collar.identifier		= randomUUID();
			await collar.save();

			console.log("new collar created", collar);
		});
	})

	socket.on('joinFrontend', () => {
		socket.join(FRONTEND_NAMESPACE);

		updateFrontendGames();
		updateFrontendCollars();
	});

	socket.on('registerGame', (data: GameData) => {
		console.log(data);

		if(!data.name || !data.humanName || !data.playerCount)
			return;

		let playerCollars = [ ];
		if(connectedGames.has(data.name)){
			playerCollars = connectedGames.get(data.name).playerCollars;
		}

		let mapdata: GameData = {...data, socket: socket, playerCollars: playerCollars}
		connectedGames.set(data.name, mapdata);

		console.log("SET DATA", mapdata);

		updateFrontendGames();
	});

	socket.on('gameSchock', (data: GameSchockData) => {
		if(!data.name || ! data.player || !data.intensity)
			return;

		let gameData = connectedGames.get(data.name);
		if(!gameData)
			return;

		let collar: entities.Collar = gameData.playerCollars[data.player];
		
		if(!collar){
			console.log("no configured collar found for player number: " + data.player +" game: "+data.name, gameData.playerCollars);
			return;
		}

		let schockIntensity = calculateShockIntensity(collar.maximumIntensity, data.intensity);
		sendSchock(schockIntensity, collar);
	});

	socket.on('assignCollar', async (collarId: number, gameName: string, playerNum: number) => {
		let game = connectedGames.get(gameName);
		if(!game)
			console.log("game '"+gameName+"' not found.");

		console.log("assign", collarId, gameName, playerNum);
		
		let collar = await entities.Collar.findOneBy({
			id: collarId
		});

		
		if(!collar)
			return;

		if(!collar.isLinked) {
			console.log("collar "+collar.identifier+" is not linked, so cannot be used for assignment");
			return;
		}
	
		game.playerCollars[playerNum] = collar;

		updateFrontendGames();
	});

	socket.on('editCollar', async (collarObj: Partial<entities.Collar>) => {
		let collar = await entities.Collar.findOneBy({
			id: collarObj.id
		});

		if(!collar)
			return;

		collar.identifier = collarObj.identifier;
		collar.maximumIntensity = parseInt(collarObj.maximumIntensity as any);
		if(collar.maximumIntensity < 0 || isNaN(collar.maximumIntensity))
			collar.maximumIntensity = 30;

			
		if(collar.maximumIntensity > 100)
			collar.maximumIntensity = 100;

		
		
		await collar.save();
		updateFrontendCollars()
	});

	socket.on('removeCollar', async (collarObj: Partial<entities.Collar>) => {
		let collar = await entities.Collar.findOneBy({
			id: collarObj.id
		});

		if(!collar)
			return;

		collar.isLinked = false;
		await collar.save();
		updateFrontendCollars()
	});

	socket.on('linkCollar', async (collarObj: Partial<entities.Collar>) => {
		let collar = await entities.Collar.findOneBy({
			id: collarObj.id
		});

		if(!collar)
			return;

		collar.isLinked = true;
		await collar.save();
		updateFrontendCollars()
	});

	socket.on('vibrateCollar', async (collarObj: Partial<entities.Collar>) => {
		let collar = await entities.Collar.findOneBy({
			id: collarObj.id
		});

		if(!collar)
			return;

		serverSock.to(SCHOCKER_NAMESPACE).emit('vibrate', {id: collar.receiverId, channel: collar.channel, strengh: 100});
	});

	socket.on('beepCollar', async (collarObj: Partial<entities.Collar>) => {
		let collar = await entities.Collar.findOneBy({
			id: collarObj.id
		});

		if(!collar)
			return;


		serverSock.to(SCHOCKER_NAMESPACE).emit('beep', {id: collar.receiverId, channel: collar.channel, strengh: 100});
	});
});

setInterval(() => {
	serverSock.emit('schock', { strength: 77, id: 12 });
}, 1000);


function updateFrontendGames(){
	console.log("Update frontend data.", connectedGames);
	let toSend = { };
	for(let g of connectedGames){
		toSend[g[0]] = g[1];
		toSend[g[0]]['socket'] = null;
	}
	serverSock.to(FRONTEND_NAMESPACE).emit('gamesUpdated', toSend);
}


async function updateFrontendCollars(){
	let collars = await entities.Collar.find();
	serverSock.to(FRONTEND_NAMESPACE).emit('collarsUpdated', collars);
}

function calculateShockIntensity(maximum: number, intensity: IntensityLevels): number{
	switch(intensity){
		case "high":
			return maximum;

		case "medium":
			return Math.floor(maximum / 2);
		
		case "low":
			return Math.floor(maximum / 3);
	}
}

function sendSchock(intensity: number, collar: entities.Collar){
	if(intensity > collar.maximumIntensity){
		console.log("tried to send a schock greater than maximum intensity. set to maximum intensity. PLEASE CHECK!")
		intensity = collar.maximumIntensity;
	}

	serverSock.to(SCHOCKER_NAMESPACE).emit('schock', {strengh: intensity, id: collar.receiverId, channel: collar.channel});
}