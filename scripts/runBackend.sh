#!/bin/bash
docker run --env DEBUG=socket.io\* -it -v ./kink8266-backend:/usr/src/app -p 8080:8080 -u 1000 -w /usr/src/app node npm run start:dev
