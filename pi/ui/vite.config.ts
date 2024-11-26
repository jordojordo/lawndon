import { fileURLToPath, URL } from 'node:url';
import os from 'os';

import { defineConfig } from 'vite';
import vue from '@vitejs/plugin-vue';
import vueDevTools from 'vite-plugin-vue-devtools';

function getLocalIP() {
  const networkInterfaces = os.networkInterfaces();

  for (const interfaceName in networkInterfaces) {
    const interfaceInfo = networkInterfaces[interfaceName];

    if (!interfaceInfo) {
      continue;
    }

    for (const info of interfaceInfo) {
      if (info.family === 'IPv4' && !info.internal) {
        return info.address;
      }
    }
  }

  return 'localhost'; // Fallback to localhost if no external IP is found
}

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    vueDevTools(),
  ],
  resolve: { alias: { '@': fileURLToPath(new URL('./src', import.meta.url)) } },
  server:  {
    proxy: { '/api': 'http://localhost:5000' },
    host:  getLocalIP()
  },
});
