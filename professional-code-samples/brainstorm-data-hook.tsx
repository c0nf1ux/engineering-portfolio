/**
 * React Hook for managing card search state and API communication.
 * Demonstrates pattern for decoupling data fetching logic from UI components.
 */

import { useState, useCallback, useEffect } from 'react';

interface SearchResult {
  id: string;
  name: string;
  manaCost: number;
  cardType: string;
}

interface UseCardSearchState {
  results: SearchResult[];
  loading: boolean;
  error: string | null;
}

export function useCardSearch(): UseCardSearchState & {
  search: (query: string, filters?: Record<string, unknown>) => Promise<void>;
} {
  const [state, setState] = useState<UseCardSearchState>({
    results: [],
    loading: false,
    error: null,
  });

  const search = useCallback(
    async (query: string, filters?: Record<string, unknown>) => {
      if (!query.trim()) {
        setState({ results: [], loading: false, error: null });
        return;
      }

      setState((prev) => ({ ...prev, loading: true, error: null }));

      try {
        const params = new URLSearchParams({ q: query });
        if (filters) {
          Object.entries(filters).forEach(([key, value]) => {
            if (value !== undefined && value !== null) {
              params.append(key, String(value));
            }
          });
        }

        const response = await fetch(`/api/cards/search?${params.toString()}`, {
          method: 'GET',
          headers: { 'Content-Type': 'application/json' },
        });

        if (!response.ok) {
          throw new Error(`Search failed: ${response.statusText}`);
        }

        const data = await response.json();
        setState({
          results: data.results || [],
          loading: false,
          error: null,
        });
      } catch (err) {
        const errorMessage =
          err instanceof Error ? err.message : 'Unknown error occurred';
        setState({
          results: [],
          loading: false,
          error: errorMessage,
        });
      }
    },
    []
  );

  return { ...state, search };
}

/**
 * Example usage in a component:
 *
 * function CardSearchComponent() {
 *   const { results, loading, error, search } = useCardSearch();
 *
 *   return (
 *     <div>
 *       <input
 *         onChange={(e) => search(e.target.value)}
 *         placeholder="Search cards..."
 *       />
 *       {loading && <p>Loading...</p>}
 *       {error && <p>Error: {error}</p>}
 *       <ul>
 *         {results.map((card) => (
 *           <li key={card.id}>{card.name}</li>
 *         ))}
 *       </ul>
 *     </div>
 *   );
 * }
 */
