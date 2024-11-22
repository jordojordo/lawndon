// / <reference types="vite/client" />

interface ImportMetaEnv {
  readonly BASE_URL: string;
  readonly PROD: boolean
}

interface ImportMeta {
  readonly env: ImportMetaEnv;
}