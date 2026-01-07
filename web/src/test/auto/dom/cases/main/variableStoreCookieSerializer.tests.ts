/*
 * Keyman is copyright (C) SIL Global. MIT License.
 */
import { assert } from 'chai';
import { VariableStore } from 'keyman/engine/keyboard';
import { VariableStoreCookieSerializer } from 'keyman/engine/main';

describe('VariableStoreCookieSerializer', () => {
  describe('findStores', () => {
    it('should return an empty array when no stores exist for a normal keyboardID', () => {

      // Arrange
      const serializer = new VariableStoreCookieSerializer();
      const keyboardID = 'test-keyboard';
      const expected: VariableStore[] = [];

      // Act
      const result = serializer.findStores(keyboardID);

      // Assert
      assert.deepEqual(result, expected, 'result should be an empty array');
      assert.isTrue(Array.isArray(result), 'result should be an array');
      assert.strictEqual(result.length, 0, 'result array length should be 0');
    });

    it('should return an empty array when no stores exist for an empty keyboardID', () => {

      // Arrange
      const serializer = new VariableStoreCookieSerializer();
      const keyboardID = '';
      const expected: VariableStore[] = [];

      // Act
      const result = serializer.findStores(keyboardID);

      // Assert
      assert.deepEqual(result, expected, 'result should be an empty array');
      assert.isTrue(Array.isArray(result), 'result should be an array');
      assert.strictEqual(result.length, 0, 'result array length should be 0');
    });

    it('should return an empty array when no stores exist for a keyboardID with special characters', () => {

      // Arrange
      const serializer = new VariableStoreCookieSerializer();
      const keyboardID = 'kbd-😀-特殊-!@#$%^&*()';
      const expected: VariableStore[] = [];

      // Act
      const result = serializer.findStores(keyboardID);

      // Assert
      assert.deepEqual(result, expected, 'result should be an empty array');
      assert.isTrue(Array.isArray(result), 'result should be an array');
      assert.strictEqual(result.length, 0, 'result array length should be 0');
    });

    it('should return a new empty array instance on each call (no shared mutable reference)', () => {

      // Arrange
      const serializer = new VariableStoreCookieSerializer();
      const keyboardID = 'any-keyboard';

      // Act
      const result1 = serializer.findStores(keyboardID);
      const result2 = serializer.findStores(keyboardID);

      // Assert
      assert.deepEqual(result1, [], 'first result should be an empty array');
      assert.deepEqual(result2, [], 'second result should be an empty array');
      assert.notStrictEqual(result1, result2, 'each call should return a new array instance');
    });

  });
});
