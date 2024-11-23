import './assets/main.css';

import { createApp } from 'vue';
import { createPinia } from 'pinia';
import { io, Socket } from 'socket.io-client';

import App from './App.vue';
import router from './router';

const app = createApp(App);

const backendUrl = import.meta.env.VITE_BACKEND_URL || 'http://localhost:5000';
const socket: Socket = io(backendUrl, { transports: ['websocket'] });

app.provide('socket', socket);

app.use(createPinia());
app.use(router);

app.mount('#app');
