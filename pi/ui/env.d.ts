// / <reference types="vite/client" />

interface ImportMetaEnv {
  readonly VITE_NODE_ENV: string;
  readonly VITE_BACKEND_URL: string;
  readonly BASE_URL: string;
  readonly PROD: boolean
}

interface ImportMeta {
  readonly env: ImportMetaEnv;
}