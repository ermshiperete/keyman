/*
 * Keyman is copyright (C) SIL Global. MIT License.
 */
import { KeyboardMinimalInterface, Keyboard, VariableStoreSerializer } from 'keyman/engine/keyboard';

export class CoreKeyboardInterface implements KeyboardMinimalInterface {
  public activeKeyboard: Keyboard;

  public constructor(public variableStoreSerializer?: VariableStoreSerializer) {
  }
}
