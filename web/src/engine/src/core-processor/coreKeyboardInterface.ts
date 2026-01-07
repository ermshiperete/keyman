/*
 * Keyman is copyright (C) SIL Global. MIT License.
 */
import { KeyboardMinimalInterface, Keyboard, VariableStoreSerializer } from 'keyman/engine/keyboard';

export class CoreKeyboardInterface implements KeyboardMinimalInterface {
  private _activeKeyboard: Keyboard;

  public get activeKeyboard(): Keyboard {
    return this._activeKeyboard;
  }
  public set activeKeyboard(keyboard: Keyboard) {
    this._activeKeyboard = keyboard;
    //this.variableStoreSerializer.loadStore()
  }

  public constructor(public variableStoreSerializer?: VariableStoreSerializer) {
  }
}
