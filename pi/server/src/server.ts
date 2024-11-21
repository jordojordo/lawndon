import express from 'express';
import { createServer } from 'http';
import { Server } from 'socket.io';
import net from 'net';
import path from 'path';

const app = express();
const httpServer = createServer(app);
const io = new Server(httpServer, {
  cors: { origin: '*' },
});

const PORT = 5000;

app.use(express.static(path.join(__dirname + '../../ui/index.html')));

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname + '../../ui/index.html'));
});

app.get('/api/config', (req, res) => {
  res.sendFile(path.join(__dirname, '../../config/anchorPositions.json'));
});

io.on('connection', (socket) => {
  console.log('A client connected');
});

const TCP_PORT = 8080;
const tcpServer = net.createServer((socket) => {
  console.log('UWB data source connected');

  socket.on('data', (data) => {
    const messages = data.toString().split('\n');

    messages.forEach((line) => {
      if (line.trim()) {
        try {
          const uwbData = JSON.parse(line);
          const uwbList = uwbData.links || [];

          io.emit('uwb_data', uwbList);
        } catch (error) {
          console.error('Invalid JSON data:', error);
        }
      }
    });
  });

  socket.on('end', () => {
    console.log('UWB data source disconnected');
  });
});

tcpServer.listen(TCP_PORT, () => {
  console.log(`TCP server listening on port ${TCP_PORT}`);
});

httpServer.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
