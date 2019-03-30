(function () {
  // If missing, inject a `<meta name="viewport">` tag to trigger YouTube's mobile layout.
  const viewport = document.head.querySelector('meta[name="viewport"]');
  if (!viewport) {
    viewport = document.createElement('meta');
    viewport.name = 'viewport';
    viewport.content = 'width=user-width, initial-scale=1';
    document.head.appendChild(viewport);
  }

  // Open the `Settings` menu.
  document.querySelector('ytp-settings-button, .ytp-settings-button').click();

  // Select the `Quality` sub-menu.
  document.querySelector('ytp-settings-menu ytp-menuitem:last-child, .ytp-settings-menu .ytp-menuitem:last-child').click();

  // Select the best `Quality`.
  document.querySelector('ytp-quality-menu ytp-menuitem:first-child, .ytp-quality-menu .ytp-menuitem:first-child').click();
})();
