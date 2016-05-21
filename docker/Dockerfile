FROM newhaircut/node-box:latest

MAINTAINER NewHaircut Team

EXPOSE 5858
EXPOSE 8000
EXPOSE 3000

ADD ./package.json $APPDIR/package.json
RUN npm install

ADD . $APPDIR

CMD npm start
