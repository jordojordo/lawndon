import './assets/main.css';

import { createApp } from 'vue';
import { createPinia } from 'pinia';
import { io, Socket } from 'socket.io-client';

import App from './App.vue';
import router from './router';

async function loadConfig() {
  const response = await fetch('/apiConfig.json');

  return response.json();
}

loadConfig().then((config) => {
  const backendUrl = config.API_URL || window.location.origin;
  const socket: Socket = io(backendUrl, { transports: ['websocket'] });

  const app = createApp(App);

  app.provide('socket', socket);

  app.use(createPinia());
  app.use(router);

  app.mount('#app');
});
