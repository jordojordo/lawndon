import './assets/main.css'

import { createApp } from 'vue'
import { createPinia } from 'pinia'
import { io, Socket } from 'socket.io-client'

import App from './App.vue'
import router from './router'

const app = createApp(App)

const socket: Socket = io('http://0.0.0.0:8080', {
  transports: ['websocket'],
})

app.provide('socket', socket)

app.use(createPinia())
app.use(router)

app.mount('#app')
