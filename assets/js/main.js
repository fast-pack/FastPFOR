// Point Prism's autoloader at the CDN so language grammars load on demand.
if (window.Prism) {
  Prism.plugins.autoloader.languages_path =
    'https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/';
}

// Subtle active-link highlight as the user scrolls through sections.
const links = [...document.querySelectorAll('.nav-links a[href^="#"]')];
const map = new Map(links.map(a => [a.getAttribute('href').slice(1), a]));
const observer = new IntersectionObserver((entries) => {
  entries.forEach(e => {
    const link = map.get(e.target.id);
    if (link && e.isIntersecting) {
      links.forEach(l => l.style.color = '');
      link.style.color = 'var(--ink)';
    }
  });
}, { rootMargin: '-45% 0px -50% 0px' });

document.querySelectorAll('main section[id]').forEach(s => observer.observe(s));
