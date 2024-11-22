import { build } from 'esbuild';

const isProduction = process.env.NODE_ENV === 'production';

build({
  entryPoints: ['./src/server.ts'],
  bundle:      true,
  platform:    'node',
  target:      'node22',
  outfile:     './dist/server.cjs', // Use .cjs extension
  sourcemap:   !isProduction,
  format:      'cjs', // Ensure format is 'cjs'
  define:      { 'process.env.NODE_ENV': `"${ process.env.NODE_ENV || 'development' }"` },
})
  .then(() => {
    console.log('Build complete');
  })
  .catch((error) => {
    console.error('Build failed:', error);
    process.exit(1);
  });
