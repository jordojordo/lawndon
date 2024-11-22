import express from 'express';
import { createServer } from 'http';
import { Server } from 'socket.io';
import net from 'net';
import { fileURLToPath } from 'url';
import { dirname } from 'path';
import path from 'path';
import cors from 'cors';


const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const httpServer = createServer(app);
const io = new Server(httpServer, {
  cors: { origin: '*' },
});

const PORT = 5000;
app.use(cors()); // Allow all origins

app.get('/api/config', (req, res) => {
  res.sendFile(path.join(__dirname, '../../config/anchorPositions.json'));
});

// Proxy to Vite dev server in development
if (process.env.NODE_ENV === 'development') {
  const { createProxyMiddleware } = await import('http-proxy-middleware');

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
  app.use(express.static(path.join(__dirname, '../../ui')));
  app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, '../../ui/index.html'));
  });
}

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
