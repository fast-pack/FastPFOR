# FastPFor website (gh-pages)

This branch holds **only** the source of the project website published at
<https://fast-pack.github.io/FastPFor/>.

It is a plain static site — no build step, no Jekyll (note the `.nojekyll` file).

```
index.html              # the page
assets/css/style.css    # styles
assets/js/main.js        # tiny progressive-enhancement script
assets/img/favicon.svg  # favicon
.github/workflows/pages.yml   # CI: deploys this branch to GitHub Pages
```

To preview locally, just open `index.html`, or run a static server:

```bash
python3 -m http.server 8000   # then visit http://localhost:8000
```

Every push to this branch redeploys the site via GitHub Actions.

> The library source code lives on the [`master`](https://github.com/fast-pack/FastPFor/tree/master) branch.
