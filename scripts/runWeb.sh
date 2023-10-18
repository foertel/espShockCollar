#!/bin/bash
docker run -it -v ./kink8266-web:/usr/src/app -p 4200:4200 -u 1000 -w /usr/src/app node npm start
