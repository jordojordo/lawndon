<script lang="ts">
import { defineComponent, onMounted, inject, ref } from 'vue';
import * as d3 from 'd3';
import { Socket } from 'socket.io-client';

interface AnchorData {
  A: string; // Anchor ID
  R: string; // Range
}

interface AnchorConfig {
  anchors: string[];
  distances: { [key: string]: number };
}

export default defineComponent({
  name: 'UWBVisualization',
  setup() {
    const socket = inject('socket') as Socket;

    const uwbData = ref<AnchorData[]>([]);
    const anchorPositions = ref<{ [key: string]: [number, number] }>({});
    const anchorDistances = ref<{ [key: string]: number }>({});
    const anchorIDs = ref<string[]>([]);

    // D3 variables
    let svg: d3.Selection<SVGSVGElement, unknown, HTMLElement, any>; // eslint-disable-line
    const width = 600;
    const height = 400;
    const padding = 20; // Padding around the visualization

    // Scales
    let xScale: d3.ScaleLinear<number, number>;
    let yScale: d3.ScaleLinear<number, number>;

    const scalingFactor = 50; // Adjust to control visualization size

    onMounted(() => {
      fetch('/api/config')
        .then((response) => response.json())
        .then((configData: AnchorConfig) => {
          console.log('## configData:', configData);
          parseConfiguration(configData);
          initVisualization();

          socket.on('uwb_data', (data: AnchorData[]) => {
            uwbData.value = data;
            updateVisualization();
            if (uwbData?.value.length === 3) {
              console.log('Incoming UWB data:', JSON.parse(JSON.stringify(uwbData.value)));
            }

          });
        })
        .catch((error) => {
          console.error('Failed to load configuration:', error);
        });
    });

    function parseConfiguration(config: AnchorConfig) {
      anchorIDs.value = config.anchors;
      anchorDistances.value = config.distances;
      computeAnchorPositions();
    }

    function computeAnchorPositions() {
      const positions: { [key: string]: [number, number] } = {};
      const ids = anchorIDs.value;

      if (ids.length < 3) {
        console.error('At least three anchors are required.');
        return;
      }


      positions[ids[0]] = [0, 0]; // First anchor at (0, 0)

      const d1 = getDistance(ids[0], ids[1]) * scalingFactor;
      positions[ids[1]] = [d1, 0]; // Second anchor on x-axis

      const d2 = getDistance(ids[0], ids[2]) * scalingFactor;
      const d3 = getDistance(ids[1], ids[2]) * scalingFactor;

      const x = (d1 ** 2 + d2 ** 2 - d3 ** 2) / (2 * d1);
      const y = Math.sqrt(Math.max(0, d2 ** 2 - x ** 2)); // Ensure no NaN from negative sqrt

      positions[ids[2]] = [x, y];

      anchorPositions.value = positions;
      updateScales();
    }

    function getDistance(id1: string, id2: string): number {
      return (
        anchorDistances.value[`${id1}-${id2}`] ||
        anchorDistances.value[`${id2}-${id1}`] ||
        0
      );
    }

    function updateScales(tagPosition?: [number, number]) {
      const allX = Object.values(anchorPositions.value).map((pos) => pos[0]);
      const allY = Object.values(anchorPositions.value).map((pos) => pos[1]);

      if (tagPosition) {
        allX.push(tagPosition[0]);
        allY.push(tagPosition[1]);
      }

      const minX = Math.min(...allX) - padding;
      const maxX = Math.max(...allX) + padding;
      const minY = Math.min(...allY) - padding;
      const maxY = Math.max(...allY) + padding;

      xScale = d3.scaleLinear().domain([minX, maxX]).range([0, width]);
      yScale = d3.scaleLinear().domain([minY, maxY]).range([height, 0]); // Invert y-axis for SVG coordinates
    }

    function initVisualization() {
      d3.select('#visualization').select('svg').remove();

      svg = d3.select('#visualization').append('svg').attr('width', width).attr('height', height);

      drawAnchors();
    }

    function drawAnchors() {
      const anchors = Object.entries(anchorPositions.value);

      // Bind data to anchor circles
      const anchorCircles = svg.selectAll('.anchor')
        .data(anchors, (d) => d[0]);

      // Enter selection
      anchorCircles.enter()
        .append('circle')
        .attr('class', 'anchor')
        .attr('r', 5)
        .attr('fill', 'green')
        .merge(anchorCircles as any) // eslint-disable-line
        .attr('cx', (d) => xScale(d[1][0]))
        .attr('cy', (d) => yScale(d[1][1]));

      // Exit selection
      anchorCircles.exit().remove();

      // Bind data to anchor labels
      const anchorLabels = svg.selectAll('.anchor-label')
        .data(anchors, (d) => d[0]);

      // Enter selection
      anchorLabels.enter()
        .append('text')
        .attr('class', 'anchor-label')
        .attr('font-size', '12px')
        .attr('fill', 'black')
        .merge(anchorLabels as any) // eslint-disable-line
        .attr('x', (d) => xScale(d[1][0]) + 5)
        .attr('y', (d) => yScale(d[1][1]) - 5)
        .text((d) => `A${d[0]}`);

      // Exit selection
      anchorLabels.exit().remove();
    }


    function updateVisualization() {
      const positions: [number, number][] = [];
      const distances: number[] = [];

      uwbData.value.forEach((anchor) => {
        const anchorID = anchor.A;
        const range = parseFloat(anchor.R) * scalingFactor; // Scale the range
        if (range <= 0) {
          console.warn(`Invalid distance for anchor ${anchorID}: ${range}. Skipping.`);
          return; // Skip invalid distances
        }
        if (anchorPositions.value[anchorID]) {
          positions.push(anchorPositions.value[anchorID]);
          distances.push(range);
        }
      });

      if (positions.length >= 3) {
        const [x, y] = calculateTagPosition(positions, distances);
        if (x !== -1 && y !== -1) {
          // Update scales to include tag position
          updateScales([x, y]);
          // Redraw anchors and tag
          drawVisualization(x, y);
        } else {
          console.error('Invalid tag position calculated.');
          svg.selectAll('.tag').remove(); // Remove tag if position is invalid
        }
      } else {
        console.error('Not enough valid anchors with positive distances.');
        svg.selectAll('.tag').remove(); // Remove tag if not enough data
      }
    }

    function drawVisualization(tagX: number, tagY: number) {
      // Clear existing anchors and tag
      svg.selectAll('.anchor').remove();
      svg.selectAll('.anchor-label').remove();
      svg.selectAll('.tag').remove();

      // Redraw anchors
      drawAnchors();

      // Draw tag
      drawTag(tagX, tagY);
    }


    function calculateTagPosition(
      positions: [number, number][],
      distances: number[]
    ): [number, number] {
      if (positions.length < 3) {
        console.error('At least three anchors are required.');
        return [-1, -1];
      }

      try {
        const [x1, y1] = positions[0];
        const [x2, y2] = positions[1];
        const [x3, y3] = positions[2];
        const r1 = distances[0];
        const r2 = distances[1];
        const r3 = distances[2];

        const A = 2 * (x2 - x1);
        const B = 2 * (y2 - y1);
        const C = r1 ** 2 - r2 ** 2 - x1 ** 2 + x2 ** 2 - y1 ** 2 + y2 ** 2;
        const D = 2 * (x3 - x1);
        const E = 2 * (y3 - y1);
        const F = r1 ** 2 - r3 ** 2 - x1 ** 2 + x3 ** 2 - y1 ** 2 + y3 ** 2;

        const denominator = A * E - B * D;

        if (denominator === 0) {
          console.error('Cannot solve, determinant is zero.');
          return [-1, -1];
        }

        const x = (C * E - B * F) / denominator;
        const y = (A * F - C * D) / denominator;

        return [x, y];
      } catch (error) {
        console.error('Error calculating position:', error);
        return [-1, -1];
      }
    }

    function drawTag(x: number, y: number) {
      svg.selectAll('.tag').remove();

      if (x !== -1 && y !== -1) {
        const scaledX = xScale(x);
        const scaledY = yScale(y);

        console.log('Tag position:', x, y);
        console.log('Scaled tag position:', scaledX, scaledY);

        svg
          .append('circle')
          .attr('class', 'tag')
          .attr('cx', scaledX)
          .attr('cy', scaledY)
          .attr('r', 5)
          .attr('fill', 'blue');
      }
    }


    return {};
  },
});
</script>

<template>
  <div id="visualization"></div>
</template>

<style scoped>
#visualization {
  border: 1px solid #ccc;
}
</style>
