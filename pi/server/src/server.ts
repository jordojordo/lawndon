import express from 'express';
import { createServer } from 'http';
import { Server } from 'socket.io';
import net from 'net';
import { fileURLToPath } from 'url';
import { resolve, join, dirname } from 'path';
import cors from 'cors';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const httpServer = createServer(app);
const io = new Server(httpServer, {
  cors: { origin: '*' },
});

const PORT = process.env.PORT || 5000;
const TCP_PORT = process.env.TCP_PORT || 8080;

const CONFIG_PATH = process.env.CONFIG_PATH || resolve(__dirname, '../../config');

app.use(cors()); // Allow all origins

app.get('/api/config', (req, res) => {
  const configPath = join(CONFIG_PATH, 'anchorPositions.json');

  res.sendFile(configPath, (err) => {
    if (err) {
      console.error('Error sending file:', err);
      res.status(500).send('Error loading configuration file.');
    }
  });
});

io.on('connection', (socket) => {
  console.log('A client connected');
});

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
