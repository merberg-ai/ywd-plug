/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        // Keep NeonPlug's existing semantic class names during the UI-only
        // checkpoint, but map them to the YWD-Hotspot visual system.
        'neon-cyan': '#62E9FF',
        'neon-cyan-bright': '#BDF7FF',
        'neon-magenta': '#FF68D4',
        'neon-magenta-bright': '#FF9BE3',
        'electric-purple': '#A979FF',
        'neon-yellow': '#FFD166',
        'dark-charcoal': '#05090D',
        'deep-gray': '#08151C',
        'cool-gray': '#7EA3AD',

        // YWD-native aliases for new components. Existing components can be
        // migrated to these incrementally instead of in one risky rewrite.
        'ywd-bg': '#05090D',
        'ywd-panel': '#08151C',
        'ywd-panel-2': '#0B1B23',
        'ywd-line': '#1D4654',
        'ywd-line-2': '#2A6678',
        'ywd-text': '#D8EDF2',
        'ywd-muted': '#7EA3AD',
        'ywd-good': '#6BF4A5',
        'ywd-warn': '#FFD166',
        'ywd-bad': '#FF6C7D',
        'ywd-magenta': '#FF68D4',
      },
      boxShadow: {
        'glow-cyan': '0 0 6px rgba(98,233,255,.65), 0 0 18px rgba(98,233,255,.28)',
        'glow-magenta': '0 0 6px rgba(255,104,212,.62), 0 0 18px rgba(255,104,212,.24)',
        'glow-purple': '0 0 6px rgba(169,121,255,.60), 0 0 18px rgba(169,121,255,.22)',
        'ywd-card': '0 12px 35px rgba(0,0,0,.32), inset 0 0 30px rgba(0,16,21,.52)',
      },
      textShadow: {
        'glow-cyan': '0 0 4px rgba(98,233,255,.75), 0 0 12px rgba(98,233,255,.35)',
        'glow-magenta': '0 0 4px rgba(255,104,212,.75), 0 0 12px rgba(255,104,212,.32)',
      },
    },
  },
  plugins: [],
}
