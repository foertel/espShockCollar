import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { SocketService } from './services/socket.service';
import { FormsModule } from '@angular/forms';


@NgModule({
	declarations: [
		AppComponent
	],
	imports: [
		BrowserModule,
		FormsModule,
		AppRoutingModule,
	],
	providers: [SocketService],
	bootstrap: [AppComponent]
})
export class AppModule { }
