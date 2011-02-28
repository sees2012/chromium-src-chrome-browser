// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('options', function() {

  var OptionsPage = options.OptionsPage;

  /**
   * FontSettings class
   * Encapsulated handling of the 'Fonts and Encoding' page.
   * @class
   */
  function FontSettings() {
    OptionsPage.call(this,
                     'fontSettings',
                     templateData.fontSettingsPageTabTitle,
                     'font-settings');
  }

  cr.addSingletonGetter(FontSettings);

  FontSettings.prototype = {
    __proto__: OptionsPage.prototype,

    /**
     * Initialize the page.
     */
    initializePage: function() {
      OptionsPage.prototype.initializePage.call(this);

      var serifFontRange = $('serif-font-size');
      serifFontRange.valueMap = $('fixed-font-size').valueMap = [9, 10, 11, 12,
          13, 14, 15, 16, 17, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 40, 44,
          48, 56, 64, 72];
      serifFontRange.continuous = false;
      serifFontRange.fontSampleEl = $('serif-font-sample');
      serifFontRange.notifyChange = this.rangeChanged_.bind(this);

      var minimumFontRange = $('minimum-font-size');
      minimumFontRange.valueMap = [9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 20,
          22, 24];
      minimumFontRange.continuous = false;
      minimumFontRange.fontSampleEl = $('minimum-font-sample');
      minimumFontRange.notifyChange = this.rangeChanged_.bind(this);
      minimumFontRange.notifyPrefChange =
          this.minimumFontSizeChanged_.bind(this);

      var placeholder = localStrings.getString('fontSettingsPlaceholder');
      $('serif-font-family').appendChild(new Option(placeholder));
      $('fixed-font-family').appendChild(new Option(placeholder));
      $('font-encoding').appendChild(new Option(placeholder));

      // Add an additional listener to the font select menu for the
      // 'sansserif_font_family' preference.
      $('serif-font-family').addEventListener('change',
          function(e) {
            Preferences.setStringPref('webkit.webprefs.sansserif_font_family',
                this.options[this.selectedIndex].value, '');
          });
    },

    /**
     * Called by the options page when this page has been shown.
     */
    didShowPage: function() {
      // The fonts list may be large so we only load it when this page is
      // loaded for the first time.  This makes opening the options window
      // faster and consume less memory if the user never opens the fonts
      // dialog.
      if (!this.hasShown) {
        chrome.send('fetchFontsData');
        this.hasShown = true;
      }
    },

    /**
     * Called as the user changes a non-continuous slider.  This allows for
     * reflecting the change in the UI before the preference has been changed.
     * @param {Element} el The slider input element.
     * @param {number} value The mapped value currently set by the slider.
     * @private
     */
    rangeChanged_: function(el, value) {
      this.setupFontSample_(el.fontSampleEl, value,
                            el.fontSampleEl.style.fontFamily);
    },

    /**
     * Sets the 'minimum_logical_font_size' preference when the minimum font
     * size has been changed by the user.
     * @param {Element} el The slider input element.
     * @param {number} value The mapped value that has been saved.
     * @private
     */
    minimumFontSizeChanged_: function(el, value) {
      Preferences.setIntegerPref('webkit.webprefs.minimum_logical_font_size',
          value, '');
    },

    /**
     * Sets the text, font size and font family of the sample text.
     * @param {Element} el The div containing the sample text.
     * @param {number} size The font size of the sample text.
     * @param {string} font The font family of the sample text.
     * @private
     */
    setupFontSample_: function(el, size, font) {
      el.textContent =
          size + "pt: " + localStrings.getString('fontSettingsLoremIpsum');
      el.style.fontSize = size + "pt";
      if (font)
        el.style.fontFamily = font;
    },

    /**
     * Populates a select list and selects the specified item.
     * @param {Element} element The select element to populate.
     * @param {Array} items The array of items from which to populate.
     * @param {string} selectedValue The selected item.
     * @private
     */
    populateSelect_: function(element, items, selectedValue) {
      // Remove any existing content.
      element.textContent = '';

      // Insert new child nodes into select element.
      var value, text, selected, option;
      for (var i = 0; i < items.length; i++) {
        value = items[i][0];
        text = items[i][1];
        selected = value == selectedValue;
        element.appendChild(new Option(text, value, false, selected));
      }

      // Enable if not a managed pref.
      if (!element.managed)
        element.disabled = false;
    }
  };

  // Chrome callbacks
  FontSettings.setFontsData = function(fonts, encodings, selectedValues) {
    FontSettings.getInstance().populateSelect_($('serif-font-family'), fonts,
                                               selectedValues[0]);
    FontSettings.getInstance().populateSelect_($('fixed-font-family'), fonts,
                                               selectedValues[1]);
    FontSettings.getInstance().populateSelect_($('font-encoding'), encodings,
                                               selectedValues[2]);
  };

  FontSettings.setupSerifFontSample = function(font, size) {
    FontSettings.getInstance().setupFontSample_($('serif-font-sample'), size,
                                                font);
  };

  FontSettings.setupFixedFontSample = function(font, size) {
    FontSettings.getInstance().setupFontSample_($('fixed-font-sample'), size,
                                                font);
  };

  FontSettings.setupMinimumFontSample = function(size) {
    FontSettings.getInstance().setupFontSample_($('minimum-font-sample'), size);
  };

  // Export
  return {
    FontSettings: FontSettings
  };
});

