import dotenv from 'dotenv';
import express from 'express';
import { createServer } from 'http';
import { Server } from 'socket.io';
import net from 'net';
import { resolve, join } from 'path';
import cors from 'cors';
import { createProxyMiddleware } from 'http-proxy-middleware';

// Load environment variables
if (process.env.NODE_ENV === 'development') {
  dotenv.config({ path: resolve(__dirname, '../.env.dev') });
} else {
  dotenv.config({ path: resolve(__dirname, '../.env.prod') });
}

const app = express();
const httpServer = createServer(app);
const io = new Server(httpServer, {
  cors: { origin: '*' },
});

const PORT = process.env.PORT || 5000;
const TCP_PORT = process.env.TCP_PORT || 8080;

// Paths for configuration and UI
const CONFIG_PATH = process.env.CONFIG_PATH || resolve(__dirname, '../../config');
const UI_PATH = process.env.UI_PATH || resolve(__dirname, '../../ui');

app.use(cors()); // Allow all origins

app.get('/api/config', (req, res) => {
  res.sendFile(join(__dirname, CONFIG_PATH, 'anchorPositions.json'));
});

// Proxy to Vite dev server in development
if (process.env.NODE_ENV === 'development') {
  app.use(
    '/',
    createProxyMiddleware({
      target: 'http://localhost:5173',
      changeOrigin: true,
      ws: true,
    })
  );
} else {
  // Serve static files in production
  app.use(express.static(UI_PATH));
  app.get('/', (req, res) => {
    res.sendFile(resolve(UI_PATH, 'index.html'));
  });
}

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
