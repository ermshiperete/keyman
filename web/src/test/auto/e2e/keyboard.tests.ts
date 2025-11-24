/*
 * Keyman is copyright (C) SIL Global. MIT License.
 */
import { test, expect, Page, Locator } from '@playwright/test';

test.describe('KMX keyboards', function () {
  let textarea: Locator;
  const beforeEach = async (page: Page) => {
    await page.goto('http://localhost:3000/src/test/manual/web/kmxkeyboard.html?keyboard=kmx');
    textarea = page.locator('#inputarea');
    await textarea.click();
    await new Promise(r => setTimeout(r, 1000));
  };

  test('can type on a (simulated) hardware keyboard', async ({ page }) => {
    await beforeEach(page);
    await page.keyboard.press('x');
    await page.keyboard.press('j');
    await page.keyboard.press('m');
    await page.keyboard.press('Shift+e');
    await page.keyboard.press('r');
    await expect(textarea).toHaveValue('ខ្មែរ', { timeout: 500 });
  });

  test('can type on a (simulated) hardware keyboard with reordering', async ({ page }) => {
    await beforeEach(page);
    await page.keyboard.press('x');
    await page.keyboard.press('Shift+e');
    await page.keyboard.press('j');
    await page.keyboard.press('m');
    await page.keyboard.press('r');
    await expect(textarea).toHaveValue('ខ្មែរ', { timeout: 500 });
  });
});

test.describe('JS keyboards', function () {
  let textarea: Locator;
  const beforeEach = async (page: Page) => {
    await page.goto('http://localhost:3000/src/test/manual/web/kmxkeyboard.html?keyboard=js');
    textarea = page.locator('#inputarea');
    await textarea.click();
    // give OSK time to appear
    await new Promise(r => setTimeout(r, 1000));
  };

  test('can type on a (simulated) hardware keyboard', async ({ page }) => {
    await beforeEach(page);
    await page.keyboard.press('x');
    await page.keyboard.press('j');
    await page.keyboard.press('m');
    await page.keyboard.press('Shift+e');
    await page.keyboard.press('r');
    await expect(textarea).toHaveValue('ខ្មែរ', { timeout: 500 });
  });

  test('can type on a (simulated) hardware keyboard with reordering', async ({ page }) => {
    await beforeEach(page);
    await page.keyboard.press('x');
    await page.keyboard.press('Shift+e');
    await page.keyboard.press('j');
    await page.keyboard.press('m');
    await page.keyboard.press('r');
    await expect(textarea).toHaveValue('ខ្មែរ', { timeout: 500 });
  });
});