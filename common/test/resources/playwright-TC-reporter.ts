/*
 * Keyman is copyright (C) SIL Global. MIT License.
 */

import type {
  FullConfig, FullResult, Reporter, Suite, TestCase, TestResult
} from '@playwright/test/reporter';

export default class PlaywrightTeamcityReporter implements Reporter {
  private flowIds: (string | number)[] = [];

  public constructor(options: { parentFlow?: string } = {}) {
    this.flowIds.push(options.parentFlow ?? 'unit_tests');
  }

  private get parentFlowId(): string | number {
    if (this.flowIds.length >= 2) {
      return this.flowIds[this.flowIds.length - 2];
    }
    return null;
  }

  private get currentFlowId(): string | number {
    return this.flowIds[this.flowIds.length - 1];
  }

  private startNewFlow(): void {
    this.flowIds.push(Math.floor(Math.random() * 100000 + 1));
    console.log(`##teamcity[flowStarted flowId='${this.currentFlowId}' parent='${this.parentFlowId}']`);
  }

  private endCurrentFlow(): void {
    console.log(`##teamcity[flowFinished flowId = '${this.flowIds.pop()}']`);
  }

  public onBegin(config: FullConfig, suite: Suite) {
  }

  public onTestBegin(test: TestCase, result: TestResult) {
    this.startNewFlow();
    console.log(`##teamcity[testStarted name='${test.titlePath()}' captureStandardOutput='true']`);
  }

  public onTestEnd(test: TestCase, result: TestResult) {
    switch (result.status) {
      case 'passed':
        console.log(`##teamcity[testFinished name='${test.titlePath()}' duration='${result.duration}']`);
        break;
      case 'failed':
      case 'interrupted':
      case 'timedOut':
        console.log(`##teamcity[testFailed name='${test.titlePath()}' message='${result.error?.message}' details='${result.error?.value ?? result.error?.cause}']`);
        break;
      case 'skipped':
        console.log(`##teamcity[testIgnored name='${test.titlePath()}' message='${result.annotations?.toString() ?? ''}']`);
        break;
    }
    this.endCurrentFlow();
  }

  public onEnd(result: FullResult) {
  }
}
